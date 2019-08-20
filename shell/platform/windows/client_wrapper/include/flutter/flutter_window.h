// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_WINDOW_H_

#include <flutter_windows.h>

#include <string>
#include <vector>

#include "plugin_registrar.h"

//#include <Windows.h>

namespace flutter {

// TODO
class FlutterViewWin32 {
 public:
  explicit FlutterViewWin32(FlutterDesktopWindowRef window) : window_(window) {}

  ~FlutterViewWin32() = default;

  // Prevent copying.
  FlutterViewWin32(FlutterViewWin32 const&) = delete;
  FlutterViewWin32& operator=(FlutterViewWin32 const&) = delete;

  // TODO
  long GetNativeWindow() { return FlutterDesktopGetHWNDFromView(window_); }

  void ProcessMessages() { FlutterDesktopProcessMessages(); }

 private:
  // Handle for interacting with the C API's window.
  //
  // Note: window_ is conceptually owned by the controller, not this object.
  FlutterDesktopWindowRef window_;
};

// A window displaying Flutter content.
class FlutterWindow {
 public:
  explicit FlutterWindow(FlutterDesktopWindowRef window) : window_(window) {}

  ~FlutterWindow() = default;

  // Prevent copying.
  FlutterWindow(FlutterWindow const&) = delete;
  FlutterWindow& operator=(FlutterWindow const&) = delete;

 private:
  // Handle for interacting with the C API's window.
  //
  // Note: window_ is conceptually owned by the controller, not this object.
  FlutterDesktopWindowRef window_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_WINDOW_H_
