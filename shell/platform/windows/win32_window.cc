// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/win32_window.h"

namespace flutter {

Win32Window::Win32Window() {
  dpi_helper_ = std::make_unique<Win32DpiHelper>();

  // Assume Windows 10 1607 for DPI handling.
  // TODO handle DPI robustly downlevel
  // TODO the calling applicaiton should participate in setting the DPI
  // awareness mode
  auto result = dpi_helper_->SetProcessDpiAwarenessContext(
      DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  if (result != TRUE) {
    OutputDebugString(L"Failed to set PMV2");
  }
}
Win32Window::~Win32Window() {
  Destroy();
}

void Win32Window::Initialize(const char* title,
                             const unsigned int x,
                             const unsigned int y,
                             const unsigned int width,
                             const unsigned int height) {
  Destroy();
  auto converted_title = NarrowToWide(title);

  auto window_class = ResgisterWindowClass(converted_title);

  CreateWindow(window_class.lpszClassName, converted_title.c_str(),
               WS_OVERLAPPEDWINDOW | WS_VISIBLE, x, y, width, height, nullptr,
               nullptr, window_class.hInstance, this);
}

std::wstring Win32Window::NarrowToWide(const char* source) {
  auto length = strlen(source);
  size_t outlen = 0;
  std::wstring wideTitle(length, L'#');
  mbstowcs_s(&outlen, &wideTitle[0], length + 1, source, length);
  return wideTitle;
}

WNDCLASS Win32Window::ResgisterWindowClass(std::wstring& title) {
  window_class_name_ = title;

  WNDCLASS window_class{};
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  window_class.lpszClassName = title.c_str();
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.cbClsExtra = 0;
  window_class.cbWndExtra = 0;
  window_class.hInstance = GetModuleHandle(nullptr);
  window_class.hIcon = nullptr;
  window_class.hbrBackground = 0;
  window_class.lpszMenuName = nullptr;
  window_class.lpfnWndProc = WndProc;
  RegisterClass(&window_class);
  return window_class;
}

LRESULT CALLBACK Win32Window::WndProc(HWND const window,
                                      UINT const message,
                                      WPARAM const wparam,
                                      LPARAM const lparam) noexcept {
  if (message == WM_NCCREATE) {
    auto cs = reinterpret_cast<CREATESTRUCT*>(lparam);
    SetWindowLongPtr(window, GWLP_USERDATA,
                     reinterpret_cast<LONG_PTR>(cs->lpCreateParams));

    auto that = static_cast<Win32Window*>(cs->lpCreateParams);

    // Since the application is running in Per-monitor V2 mode, turn on automatic titlebar
    // scaling
    auto result = that->dpi_helper_->EnableNonClientDpiScaling(window);
    if (result != TRUE) {
      OutputDebugString(L"Failed to enable non-client area autoscaling");
    }
    that->current_dpi_ = that->dpi_helper_->GetDpiForWindow(window);
    that->window_handle_ = window;
  } else if (Win32Window* that = GetThisFromHandle(window)) {
    return that->MessageHandler(window, message, wparam, lparam);
  }

  return DefWindowProc(window, message, wparam, lparam);
}

LRESULT
Win32Window::MessageHandler(HWND hwnd,
                            UINT const message,
                            WPARAM const wparam,
                            LPARAM const lparam) noexcept {
  int xPos = 0, yPos = 0;
  UINT width = 0, height = 0;
  auto window =
      reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

  if (window != nullptr) {
    switch (message) {
      case WM_DPICHANGED:
        return HandleDpiChange(window_handle_, wparam, lparam);
        break;

      case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        break;

      case WM_SIZE:
        width = LOWORD(lparam);
        height = HIWORD(lparam);

        current_width_ = width;
        current_height_ = height;
        window->OnResize(width, height);
        break;

      case WM_MOUSEMOVE:
        xPos = GET_X_LPARAM(lparam);
        yPos = GET_Y_LPARAM(lparam);

        window->OnPointerMove(static_cast<double>(xPos),
                              static_cast<double>(yPos));
        break;
      case WM_LBUTTONDOWN:
        xPos = GET_X_LPARAM(lparam);
        yPos = GET_Y_LPARAM(lparam);
        window->OnPointerDown(static_cast<double>(xPos),
                              static_cast<double>(yPos));
        break;
      case WM_LBUTTONUP:
        xPos = GET_X_LPARAM(lparam);
        yPos = GET_Y_LPARAM(lparam);
        window->OnPointerUp(static_cast<double>(xPos),
                            static_cast<double>(yPos));
        break;
      case WM_MOUSEWHEEL:
        window->OnScroll(
            0.0, -(static_cast<short>(HIWORD(wparam)) / (double)WHEEL_DELTA));
        break;
      case WM_CHAR:
      case WM_SYSCHAR:
      case WM_UNICHAR:
        if (wparam != VK_BACK) {
          window->OnChar(static_cast<unsigned int>(wparam));
        }
        break;
      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
      case WM_KEYUP:
      case WM_SYSKEYUP:
        unsigned char scancode = ((unsigned char*)&lparam)[2];
        unsigned int virtualKey = MapVirtualKey(scancode, MAPVK_VSC_TO_VK);
        const int key = virtualKey;
        const int action = message == WM_KEYDOWN ? WM_KEYDOWN : WM_KEYUP;
        window->OnKey(key, scancode, action, 0);
        break;
    }
    return DefWindowProc(hwnd, message, wparam, lparam);
  }

  return DefWindowProc(window_handle_, message, wparam, lparam);
}  // namespace flutter

void Win32Window::Destroy() {
  if (window_handle_) {
    DestroyWindow(window_handle_);
    window_handle_ = nullptr;
  }

  UnregisterClass(window_class_name_.c_str(), nullptr);
}

// DPI Change handler. on WM_DPICHANGE resize the window
LRESULT
Win32Window::HandleDpiChange(HWND hwnd, WPARAM wparam, LPARAM lparam) {
  if (hwnd != nullptr) {
    auto window =
        reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    UINT uDpi = HIWORD(wparam);

    window->OnDpiScale(uDpi);

    // Resize the window
    auto lprcNewScale = reinterpret_cast<RECT*>(lparam);
    auto newWidth = lprcNewScale->right - lprcNewScale->left;
    auto newHeight = lprcNewScale->bottom - lprcNewScale->top;

    SetWindowPos(hwnd, nullptr, lprcNewScale->left, lprcNewScale->top, newWidth,
                 newHeight, SWP_NOZORDER | SWP_NOACTIVATE);
  }
  return 0;
}

Win32Window* Win32Window::GetThisFromHandle(HWND const window) noexcept {
  return reinterpret_cast<Win32Window*>(
      GetWindowLongPtr(window, GWLP_USERDATA));
}

}  // namespace flutter