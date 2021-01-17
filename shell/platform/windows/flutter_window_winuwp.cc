// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/flutter_window_winuwp.h"

namespace flutter {

FlutterWindowWinUWP::FlutterWindowWinUWP(
    ABI::Windows::UI::Core::CoreWindow* window)
    : game_controller_thread_running_(false),
      running_on_xbox_(false),
      xbox_overscan_x_offset_(0),
      xbox_overscan_y_offset_(0),
      current_display_info_(nullptr) {
  winrt::Windows::UI::Core::CoreWindow cw{nullptr};
  winrt::copy_from_abi(cw, window);
  window_ = cw;

  SetEventHandlers();
  SetupGamePad();
  SetupXboxSpecific();

  current_display_info_ = winrt::Windows::Graphics::Display::
      DisplayInformation::GetForCurrentView();
}

WindowsRenderTarget FlutterWindowWinUWP::GetRenderTarget() {
#ifdef USECOREWINDOW
  return WindowsRenderTarget(window_);
#else
  compositor_ = winrt::Windows::UI::Composition::Compositor();
  target_ = compositor_.CreateTargetForCurrentView();
  visual_tree_root_ = compositor_.CreateContainerVisual();
  target_.Root(visual_tree_root_);

  cursor_visual_ = CreateCursorVisual();

  if (game_controller_thread_running_) {
    visual_tree_root_.Children().InsertAtTop(cursor_visual_);
  }

  sprite_visual_ = compositor_.CreateSpriteVisual();
  if (running_on_xbox_) {
    sprite_visual_.Offset(
        {xbox_overscan_x_offset_, xbox_overscan_y_offset_, 1.0});
  } else {
    sprite_visual_.Offset({1.0, 1.0, 1.0});
    ApplyInverseDpiScalingTransform();
  }
  visual_tree_root_.Children().InsertAtBottom(sprite_visual_);

  auto bounds = GetBounds(current_display_info_, true);

  sprite_visual_.Size({bounds.width, bounds.height});
  return WindowsRenderTarget(sprite_visual_);
#endif
}

void FlutterWindowWinUWP::ApplyInverseDpiScalingTransform() {
  // apply inverse transform to negate built in DPI scaling so we can render at
  // native scale
  auto dpiScale = GetDpiScale();
  sprite_visual_.Scale({1 / dpiScale, 1 / dpiScale, 1 / dpiScale});
}

PhysicalWindowBounds FlutterWindowWinUWP::GetPhysicalWindowBounds() {
  auto bounds = GetBounds(current_display_info_, true);
  return {static_cast<size_t>(bounds.width),
          static_cast<size_t>(bounds.height)};
}

void FlutterWindowWinUWP::UpdateFlutterCursor(const std::string& cursor_name) {
  // TODO
}

float FlutterWindowWinUWP::GetDpiScale() {
  auto disp = winrt::Windows::Graphics::Display::DisplayInformation::
      GetForCurrentView();

  return GetDpiScale(disp);
}

WindowBoundsWinUWP FlutterWindowWinUWP::GetBounds(
    winrt::Windows::Graphics::Display::DisplayInformation const& disp,
    bool physical) {
  auto appView =
      winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
  auto bounds = appView.VisibleBounds();
  if (running_on_xbox_) {
    return {bounds.Width + (bounds.X), bounds.Height + (bounds.Y)};
  }

  if (physical) {
    // Return the height in physical pixels
    return {bounds.Width * static_cast<float>(disp.RawPixelsPerViewPixel()),
            bounds.Height * static_cast<float>(disp.RawPixelsPerViewPixel())};
  }

  return {bounds.Width, bounds.Height};
}

float FlutterWindowWinUWP::GetDpiScale(
    winrt::Windows::Graphics::Display::DisplayInformation const& disp) {
  float dpi = disp.LogicalDpi();
  auto rawdpi = disp.RawDpiX();
  auto rawwidth = disp.ScreenHeightInRawPixels();
  auto scale = disp.ResolutionScale();
  auto rawperview = disp.RawPixelsPerViewPixel();

  // because XBOX has display scaling off, logicalDpi retuns 96 which is
  // incorrect
  // TODO check if rawperview is more acurate
  if (running_on_xbox_) {
    return 1.5;
  }
  // TODO do we need this on 10X?
  // return dpi/96.0 * 0.8;
  return static_cast<float>(rawperview);
}

FlutterWindowWinUWP::~FlutterWindowWinUWP() {
  OutputDebugString(L"Destoryed UWP FlutterWindow");
}

void FlutterWindowWinUWP::SetView(WindowBindingHandlerDelegate* view) {
  binding_handler_delegate_ = view;
}

void FlutterWindowWinUWP::SetEventHandlers() {
  auto appView =
      winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();

  appView.SetDesiredBoundsMode(winrt::Windows::UI::ViewManagement::
                                   ApplicationViewBoundsMode::UseCoreWindow);

  appView.VisibleBoundsChanged({this, &FlutterWindowWinUWP::OnBoundsChanged});

  window_.PointerPressed({this, &FlutterWindowWinUWP::OnPointerPressed});
  window_.PointerReleased({this, &FlutterWindowWinUWP::OnPointerReleased});
  window_.PointerMoved({this, &FlutterWindowWinUWP::OnPointerMoved});
  window_.PointerWheelChanged(
      {this, &FlutterWindowWinUWP::OnPointerWheelChanged});

  // TODO mouse leave
  // TODO fontchanged

  window_.KeyUp({this, &FlutterWindowWinUWP::OnKeyUp});
  window_.KeyDown({this, &FlutterWindowWinUWP::OnKeyDown});
  window_.CharacterReceived({this, &FlutterWindowWinUWP::OnCharacterReceived});

  auto disp = winrt::Windows::Graphics::Display::DisplayInformation::
      GetForCurrentView();
  disp.DpiChanged({this, &FlutterWindowWinUWP::OnDpiChanged});
}

void FlutterWindowWinUWP::StartGamepadCursorThread() {
  if (worker_loop_ != nullptr &&
      worker_loop_.Status() ==
          winrt::Windows::Foundation::AsyncStatus::Started) {
    return;
  }

  winrt::Windows::UI::Core::CoreDispatcher dispatcher = window_.Dispatcher();

  auto workItemHandler = winrt::Windows::System::Threading::WorkItemHandler(
      [this, dispatcher](winrt::Windows::Foundation::IAsyncAction action) {
        while (action.Status() ==
               winrt::Windows::Foundation::AsyncStatus::Started) {
          dispatcher.RunAsync(
              winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
              [this, dispatcher]() { game_pad_->Process(); });
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });

  worker_loop_ = winrt::Windows::System::Threading::ThreadPool::RunAsync(
      workItemHandler, winrt::Windows::System::Threading::WorkItemPriority::Low,
      winrt::Windows::System::Threading::WorkItemOptions::TimeSliced);
}

void FlutterWindowWinUWP::SetupGamePad() {
  DualAxisCallback leftStick = [=](double x, double y) {
    OnGamePadLeftStickMoved(x, y);
  };

  DualAxisCallback rightStick = [=](double x, double y) {
    OnGamePadRightStickMoved(x, y);
  };

  ButtonCallback pressedcallback =
      [=](winrt::Windows::Gaming::Input::GamepadButtons buttons) {
        OnGamePadButtonPressed(buttons);
      };

  ButtonCallback releasedcallback =
      [=](winrt::Windows::Gaming::Input::GamepadButtons buttons) {
        OnGamePadButtonReleased(buttons);
      };
  GamepadAddedRemovedCallback changed = [=]() {
    OnGamePadControllersChanged();
  };

  game_pad_ = std::make_unique<GamePadWinUWP>(leftStick, rightStick, nullptr,
                                             nullptr, pressedcallback,
                                             releasedcallback, changed);
  game_pad_->Initialize();
}

void FlutterWindowWinUWP::SetupXboxSpecific() {
  running_on_xbox_ =
      winrt::Windows::System::Profile::AnalyticsInfo::VersionInfo()
          .DeviceFamily() == L"Windows.Xbox";

  if (running_on_xbox_) {
    bool result =
        winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()
            .SetDesiredBoundsMode(winrt::Windows::UI::ViewManagement::
                                      ApplicationViewBoundsMode::UseCoreWindow);
    if (!result) {
      OutputDebugString(L"Couldn't set bounds mode.");
    }

    auto appView = winrt::Windows::UI::ViewManagement::ApplicationView::
        GetForCurrentView();
    auto bounds = appView.VisibleBounds();

    // the offset /2 represents how much off-screan the core window is
    // positioned unclear why disabling overscan doesn't correct this
    xbox_overscan_x_offset_ = bounds.X / 2;
    xbox_overscan_y_offset_ = bounds.Y / 2;
  }
}

void FlutterWindowWinUWP::OnDpiChanged(
    winrt::Windows::Graphics::Display::DisplayInformation const& args,
    winrt::Windows::Foundation::IInspectable const&) {
  ApplyInverseDpiScalingTransform();

  auto bounds = GetBounds(current_display_info_, true);

  binding_handler_delegate_->OnWindowSizeChanged(
      static_cast<size_t>(bounds.width), static_cast<size_t>(bounds.height));
}

void FlutterWindowWinUWP::OnPointerPressed(
    winrt::Windows::Foundation::IInspectable const&,
    winrt::Windows::UI::Core::PointerEventArgs const& args) {
  auto x = GetPosX(args);
  auto y = GetPosY(args);

  binding_handler_delegate_->OnPointerDown(
      x, y, FlutterPointerMouseButtons::kFlutterPointerButtonMousePrimary);
}

void FlutterWindowWinUWP::OnPointerReleased(
    winrt::Windows::Foundation::IInspectable const&,
    winrt::Windows::UI::Core::PointerEventArgs const& args) {
  auto x = GetPosX(args);
  auto y = GetPosY(args);

  binding_handler_delegate_->OnPointerUp(
      x, y, FlutterPointerMouseButtons::kFlutterPointerButtonMousePrimary);
}

void FlutterWindowWinUWP::OnPointerMoved(
    winrt::Windows::Foundation::IInspectable const&,
    winrt::Windows::UI::Core::PointerEventArgs const& args) {
  auto x = GetPosX(args);
  auto y = GetPosY(args);

  binding_handler_delegate_->OnPointerMove(x, y);
}

void FlutterWindowWinUWP::OnPointerWheelChanged(
    winrt::Windows::Foundation::IInspectable const&,
    winrt::Windows::UI::Core::PointerEventArgs const& args) {
  auto x = GetPosX(args);
  auto y = GetPosY(args);
  auto delta = args.CurrentPoint().Properties().MouseWheelDelta();
  binding_handler_delegate_->OnScroll(x, y, 0, -delta, 1);
}

double FlutterWindowWinUWP::GetPosX(
    winrt::Windows::UI::Core::PointerEventArgs const& args) {
  // TODO cache this
  const double inverseDpiScale = GetDpiScale();

  return static_cast<double>(
      (args.CurrentPoint().Position().X - xbox_overscan_x_offset_) *
      inverseDpiScale);
}

double FlutterWindowWinUWP::GetPosY(
    winrt::Windows::UI::Core::PointerEventArgs const& args) {
  // TODO cache this
  const double inverseDpiScale = GetDpiScale();
  return static_cast<double>(
      (args.CurrentPoint().Position().Y - xbox_overscan_y_offset_) *
      inverseDpiScale);
}

void FlutterWindowWinUWP::OnBoundsChanged(
    winrt::Windows::UI::ViewManagement::ApplicationView const& appView,
    winrt::Windows::Foundation::IInspectable const&) {
  if (binding_handler_delegate_) {
#ifndef USECOREWINDOW
    auto bounds = GetBounds(current_display_info_, true);

    binding_handler_delegate_->OnWindowSizeChanged(
        static_cast<size_t>(bounds.width), static_cast<size_t>(bounds.height));
    sprite_visual_.Size({bounds.width, bounds.height});
#endif
  }
}

void FlutterWindowWinUWP::OnKeyUp(
    winrt::Windows::Foundation::IInspectable const&,
    winrt::Windows::UI::Core::KeyEventArgs const& args) {
  // TODO: system key (back) and unicode handling
  auto status = args.KeyStatus();
  unsigned int scancode = status.ScanCode;
  int key = static_cast<int>(args.VirtualKey());
  char32_t chararacter = static_cast<char32_t>(key | 32);
  int action = 0x0101;
  binding_handler_delegate_->OnKey(key, scancode, action, chararacter);
}

void FlutterWindowWinUWP::OnKeyDown(
    winrt::Windows::Foundation::IInspectable const&,
    winrt::Windows::UI::Core::KeyEventArgs const& args) {
  // TODO: system key (back) and unicode handling
  auto status = args.KeyStatus();
  unsigned int scancode = status.ScanCode;
  int key = static_cast<int>(args.VirtualKey());
  char32_t chararacter = static_cast<char32_t>(key | 32);
  int action = 0x0100;
  binding_handler_delegate_->OnKey(key, scancode, action, chararacter);
}

void FlutterWindowWinUWP::OnCharacterReceived(
    winrt::Windows::Foundation::IInspectable const&,
    winrt::Windows::UI::Core::CharacterReceivedEventArgs const& args) {
  auto key = args.KeyCode();
  wchar_t keycode = static_cast<wchar_t>(key);
  if (keycode >= u' ') {
    std::u16string text({keycode});
    binding_handler_delegate_->OnText(text);
  }
}

void FlutterWindowWinUWP::OnGamePadLeftStickMoved(double x, double y) {
  static const int CURSORSCALE = 30;

  auto newx = cursor_visual_.Offset().x + (CURSORSCALE * static_cast<float>(x));

  auto newy =
      cursor_visual_.Offset().y + (CURSORSCALE * -static_cast<float>(y));

  auto logicalBounds = GetBounds(current_display_info_, false);

  if (newx > 0 && newy > 0 && newx < logicalBounds.width &&
      newy < logicalBounds.height) {
    cursor_visual_.Offset({newx, newy, 0});
    if (!running_on_xbox_) {
      const double inverseDpiScale = GetDpiScale();
      binding_handler_delegate_->OnPointerMove(
          cursor_visual_.Offset().x * inverseDpiScale,
          cursor_visual_.Offset().y * inverseDpiScale);
    } else {
      binding_handler_delegate_->OnPointerMove(cursor_visual_.Offset().x,
                                               cursor_visual_.Offset().y);
    }
  }
}

void FlutterWindowWinUWP::OnGamePadRightStickMoved(double x, double y) {
  static const double controllerScrollMultiplier = 3;

  if (!running_on_xbox_) {
    const double inverseDpiScale = GetDpiScale();

    binding_handler_delegate_->OnScroll(
        cursor_visual_.Offset().x * inverseDpiScale,
        cursor_visual_.Offset().y * inverseDpiScale,
        x * controllerScrollMultiplier, y * controllerScrollMultiplier, 1);
  } else {
    binding_handler_delegate_->OnScroll(
        cursor_visual_.Offset().x, cursor_visual_.Offset().y,
        x * controllerScrollMultiplier, y * controllerScrollMultiplier, 1);
  }
}

void FlutterWindowWinUWP::OnGamePadButtonPressed(
    winrt::Windows::Gaming::Input::GamepadButtons buttons) {
  if ((buttons & winrt::Windows::Gaming::Input::GamepadButtons::A) ==
      winrt::Windows::Gaming::Input::GamepadButtons::A) {
    if (!running_on_xbox_) {
      const double inverseDpiScale = GetDpiScale();
      binding_handler_delegate_->OnPointerDown(
          cursor_visual_.Offset().x * inverseDpiScale,
          cursor_visual_.Offset().y * inverseDpiScale,
          FlutterPointerMouseButtons::kFlutterPointerButtonMousePrimary);
    } else {
      binding_handler_delegate_->OnPointerDown(
          cursor_visual_.Offset().x, cursor_visual_.Offset().y,
          FlutterPointerMouseButtons::kFlutterPointerButtonMousePrimary);
    }
  }
}

void FlutterWindowWinUWP::OnGamePadButtonReleased(
    winrt::Windows::Gaming::Input::GamepadButtons buttons) {
  if ((buttons & winrt::Windows::Gaming::Input::GamepadButtons::A) ==
      winrt::Windows::Gaming::Input::GamepadButtons::A) {
    if (!running_on_xbox_) {
      const double inverseDpiScale = GetDpiScale();

      binding_handler_delegate_->OnPointerUp(
          cursor_visual_.Offset().x * inverseDpiScale,
          cursor_visual_.Offset().y * inverseDpiScale,
          FlutterPointerMouseButtons::kFlutterPointerButtonMousePrimary);
    } else {
      binding_handler_delegate_->OnPointerUp(
          cursor_visual_.Offset().x, cursor_visual_.Offset().y,
          FlutterPointerMouseButtons::kFlutterPointerButtonMousePrimary);
    }
  }
}

void FlutterWindowWinUWP::OnGamePadControllersChanged() {
  // TODO lock here
  if (game_pad_->HasController()) {
    if (!game_controller_thread_running_) {
      if (cursor_visual_ != nullptr) {
        visual_tree_root_.Children().InsertAtTop(cursor_visual_);
      }
      StartGamepadCursorThread();
      game_controller_thread_running_ = true;
    } else {
      if (cursor_visual_ != nullptr) {
        visual_tree_root_.Children().Remove(cursor_visual_);
      }
      game_controller_thread_running_ = false;
      // TODO stop game thread
    }
  }
}

winrt::Windows::UI::Composition::Visual
FlutterWindowWinUWP::CreateCursorVisual() {
  auto container = compositor_.CreateContainerVisual();
  container.Offset(
      {window_.Bounds().Width / 2, window_.Bounds().Height / 2, 1.0});

  const float SIZE = 30;
  auto cursor_visual = compositor_.CreateShapeVisual();
  cursor_visual.Size({SIZE, SIZE});

  // compensate for overscan in cursor visual
  cursor_visual.Offset({xbox_overscan_x_offset_, xbox_overscan_y_offset_, 1.0});

  winrt::Windows::UI::Composition::CompositionEllipseGeometry circle =
      compositor_.CreateEllipseGeometry();
  circle.Radius({SIZE / 2, SIZE / 2});

  auto circleshape = compositor_.CreateSpriteShape(circle);
  circleshape.FillBrush(
      compositor_.CreateColorBrush(winrt::Windows::UI::Colors::Black()));
  circleshape.Offset({SIZE / 2, SIZE / 2});

  cursor_visual.Shapes().Append(circleshape);

  winrt::Windows::UI::Composition::Visual visual =
      cursor_visual.as<winrt::Windows::UI::Composition::Visual>();

  visual.CompositeMode(winrt::Windows::UI::Composition::
                           CompositionCompositeMode::DestinationInvert);

  visual.AnchorPoint({0.5, 0.5});
  container.Children().InsertAtTop(visual);

  return container;
}

}  // namespace flutter
