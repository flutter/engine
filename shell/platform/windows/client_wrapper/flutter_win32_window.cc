#include "include/flutter/flutter_win32_window.h"

#include <windows.h>

namespace flutter {

FlutterWin32Window::FlutterWin32Window(std::shared_ptr<FlutterEngine> engine)
    : engine_{std::move(engine)} {}

FlutterWin32Window::FlutterWin32Window(std::shared_ptr<FlutterEngine> engine,
                                       std::shared_ptr<Win32Wrapper> wrapper)
    : engine_{std::move(engine)}, Win32Window{std::move(wrapper)} {}

auto FlutterWin32Window::GetFlutterViewId() const -> FlutterViewId {
  return view_controller_->view_id();
};

auto FlutterWin32Window::OnCreate() -> bool {
  if (!Win32Window::OnCreate()) {
    return false;
  }

  auto const client_rect{GetClientArea()};

  // The size here must match the window dimensions to avoid unnecessary surface
  // creation / destruction in the startup path.
  view_controller_ = std::make_unique<FlutterViewController>(
      client_rect.right - client_rect.left,
      client_rect.bottom - client_rect.top, engine_);
  // Ensure that basic setup of the controller was successful.
  if (!view_controller_->view()) {
    return false;
  }

  SetChildContent(view_controller_->view()->GetNativeWindow());

  // TODO(loicsharma): Hide the window until the first frame is rendered.
  // Single window apps use the engine's next frame callback to show the window.
  // This doesn't work for multi window apps as the engine cannot have multiple
  // next frame callbacks. If multiple windows are created, only the last one
  // will be shown.
  return true;
}

void FlutterWin32Window::OnDestroy() {
  if (view_controller_) {
    view_controller_ = nullptr;
  }

  Win32Window::OnDestroy();
}

auto FlutterWin32Window::MessageHandler(HWND hwnd,
                                        UINT const message,
                                        WPARAM const wparam,
                                        LPARAM const lparam) -> LRESULT {
  // Give Flutter, including plugins, an opportunity to handle window messages.
  if (view_controller_) {
    auto const result{view_controller_->HandleTopLevelWindowProc(
        hwnd, message, wparam, lparam)};
    if (result) {
      return *result;
    }
  }

  switch (message) {
    case WM_FONTCHANGE:
      engine_->ReloadSystemFonts();
      break;
    default:
      break;
  }

  return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}

}  // namespace flutter
