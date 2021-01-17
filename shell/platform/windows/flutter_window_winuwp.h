// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_UWP_FLUTTER_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_UWP_FLUTTER_WINDOW_H_

#include "flutter/shell/platform/windows/flutter_windows_view.h"
#include "flutter/shell/platform/windows/game_pad_winuwp.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"
#include "flutter/shell/platform/windows/window_binding_handler.h"

#include "flutter/shell/platform/embedder/embedder.h"

#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.System.Profile.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.UI.ViewManagement.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include "winrt/Windows.System.Threading.h"
#include "winrt/Windows.UI.Core.h"

#include <windows.ui.core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Core.h>

namespace flutter {

struct WindowBoundsWinUWP {
  float width;
  float height;
};

// Implements a UWP CoreWindow.  Underlying window has been created and provided
// by the runner.
//
// Specifically handles window events within windows.
class FlutterWindowWinUWP : public WindowBindingHandler {
 public:
  explicit FlutterWindowWinUWP(ABI::Windows::UI::Core::CoreWindow* window);

  virtual ~FlutterWindowWinUWP();

  // |SetView|
  void SetView(WindowBindingHandlerDelegate* view) override;

  // |GetRenderTarget|
  WindowsRenderTarget GetRenderTarget() override;

  // |GetDpiScale|
  float GetDpiScale() override;

  // |GetPhysicalWindowBounds|
  PhysicalWindowBounds GetPhysicalWindowBounds() override;

  // |UpdateFlutterCursor|
  void UpdateFlutterCursor(const std::string& cursor_name) override;

 private:
  WindowBoundsWinUWP GetBounds(
      winrt::Windows::Graphics::Display::DisplayInformation const& disp,
      bool physical);

  float GetDpiScale(
      winrt::Windows::Graphics::Display::DisplayInformation const&);

  void ApplyInverseDpiScalingTransform();

  void SetEventHandlers();

  void OnDpiChanged(
      winrt::Windows::Graphics::Display::DisplayInformation const& args,
      winrt::Windows::Foundation::IInspectable const&);

  void OnPointerPressed(winrt::Windows::Foundation::IInspectable const&,
                        winrt::Windows::UI::Core::PointerEventArgs const& args);

  void OnPointerReleased(
      winrt::Windows::Foundation::IInspectable const&,
      winrt::Windows::UI::Core::PointerEventArgs const& args);

  void OnBoundsChanged(
      winrt::Windows::UI::ViewManagement::ApplicationView const& appView,
      winrt::Windows::Foundation::IInspectable const&);

  void OnPointerMoved(winrt::Windows::Foundation::IInspectable const&,
                      winrt::Windows::UI::Core::PointerEventArgs const& args);

  void OnPointerWheelChanged(
      winrt::Windows::Foundation::IInspectable const&,
      winrt::Windows::UI::Core::PointerEventArgs const& args);

  void OnKeyUp(winrt::Windows::Foundation::IInspectable const&,
               winrt::Windows::UI::Core::KeyEventArgs const& args);

  void OnKeyDown(winrt::Windows::Foundation::IInspectable const&,
                 winrt::Windows::UI::Core::KeyEventArgs const& args);

  void OnCharacterReceived(
      winrt::Windows::Foundation::IInspectable const&,
      winrt::Windows::UI::Core::CharacterReceivedEventArgs const& args);

  void OnGamePadLeftStickMoved(double x, double y);

  void OnGamePadRightStickMoved(double x, double y);

  void OnGamePadButtonPressed(
      winrt::Windows::Gaming::Input::GamepadButtons buttons);

  void OnGamePadButtonReleased(
      winrt::Windows::Gaming::Input::GamepadButtons buttons);

  void OnGamePadControllersChanged();

  winrt::Windows::UI::Composition::Visual CreateCursorVisual();

  void StartGamepadCursorThread();

  void SetupGamePad();

  void SetupXboxSpecific();

  double GetPosX(winrt::Windows::UI::Core::PointerEventArgs const& args);
  double GetPosY(winrt::Windows::UI::Core::PointerEventArgs const& args);

  winrt::Windows::Foundation::IAsyncAction worker_loop_{nullptr};

  // The backing CoreWindow. nullptr if not set.
  winrt::Windows::UI::Core::CoreWindow window_{nullptr};

  // A pointer to a FlutterWindowsView that can be used to update engine
  // windowing and input state.
  WindowBindingHandlerDelegate* binding_handler_delegate_;

  winrt::Windows::UI::Composition::Compositor compositor_{nullptr};

  winrt::Windows::UI::Composition::CompositionTarget target_{nullptr};

  winrt::Windows::UI::Composition::ContainerVisual visual_tree_root_{nullptr};

  winrt::Windows::UI::Composition::Visual cursor_visual_{nullptr};

  winrt::Windows::UI::Composition::SpriteVisual sprite_visual_{nullptr};

  std::unique_ptr<GamePadWinUWP> game_pad_{nullptr};

  bool game_controller_thread_running_;

  bool running_on_xbox_;

  float xbox_overscan_x_offset_;
  float xbox_overscan_y_offset_;

  winrt::Windows::Graphics::Display::DisplayInformation current_display_info_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_UWP_FLUTTER_WINDOW_H_
