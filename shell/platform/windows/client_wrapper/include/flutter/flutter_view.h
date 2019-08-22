// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_H_

#include <flutter_windows.h>

#include <memory>
#include <string>
#include <vector>

#include "plugin_registrar.h"

namespace flutter {

class FlutterViewWin32;

typedef std::shared_ptr<FlutterViewWin32> FlutterView;

// A view for displaying Flutter content.
class FlutterViewWin32 {
 public:
  explicit FlutterViewWin32(FlutterDesktopViewRef view) : view_(view) {}

  ~FlutterViewWin32() = default;

  // Prevent copying.
  FlutterViewWin32(FlutterViewWin32 const&) = delete;
  FlutterViewWin32& operator=(FlutterViewWin32 const&) = delete;

  // Return backing HWND for manipulation in host application.
  long GetNativeWindow() { return FlutterDesktopGetHWNDFromView(view_); }

  void ProcessMessages() { FlutterDesktopProcessMessages(); }

 private:
  // Handle for interacting with the C API's window.
  //
  // Note: view_ is conceptually owned by the controller, not this object.
  FlutterDesktopViewRef view_;
};

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_H_
}