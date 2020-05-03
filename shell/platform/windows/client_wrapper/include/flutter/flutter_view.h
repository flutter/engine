// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_H_

//#include <memory>

#include <flutter_windows.h>
#include "win32_flutter_window_pub.h"

namespace flutter {

// A view displaying Flutter content.
class FlutterView {
 public:
  explicit FlutterView(
      FlutterDesktopViewRef view
      )
      : view_(view) {
  }

  virtual ~FlutterView() = default;

  // Prevent copying.
  FlutterView(FlutterView const&) = delete;
  FlutterView& operator=(FlutterView const&) = delete;

  // Returns the backing HWND for the view.
  HWND GetNativeWindow() {
    void* windowptr = V2FlutterDesktopGetExternalWindow(view_);
    Win32FlutterWindowPub* ptr = static_cast<Win32FlutterWindowPub*>(windowptr);
    return ptr->GetWindowHandle();
  }

  void SendWindowMetrics(size_t width, size_t height, double dpiScale) {
    V2FlutterDesktopSendWindowMetrics(view_, width, height, dpiScale);
  }

  void SendPointerMove(double x, double y) {
    V2FlutterDesktopSendPointerMove(view_, x, y);
  }

  void SendPointerDown(double x, double y, uint64_t btn) {
    V2FlutterDesktopSendPointerDown(view_, x, y, btn);
  }

  void SendPointerUp(double x, double y, uint64_t btn) {
    V2FlutterDesktopSendPointerUp(view_, x, y, btn);
  }

  void SendPointerLeave() { V2FlutterDesktopSendPointerLeave(view_);
  }

  void SendScroll(double x, double y, double delta_x, double delta_y) {
    V2FlutterDesktopSendScroll(view_, x, y, delta_x, delta_y);
  }

  void SendFontChange() {
    V2FlutterDesktopSendFontChange(view_);
  }

  void SendText(const char16_t* code_point, size_t size) {
    V2FlutterDesktopSendText(view_, code_point, size);
  }

  void SendKey(int key, int scancode, int action, char32_t character) {
    V2FlutterDesktopSendKey(view_, key, scancode, action, character);
  }


 private:
  // Handle for interacting with the C API's view.
  FlutterDesktopViewRef view_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_H_
