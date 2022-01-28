// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_H_

#include <flutter_windows.h>

#include "flutter_task_runner.h"

namespace flutter {

// A view displaying Flutter content.
class FlutterView {
 public:
  explicit FlutterView(FlutterDesktopViewRef view)
      : view_(view),
        platform_task_runner_(std::make_unique<FlutterTaskRunner>(
            FlutterDesktopViewGetTaskRunner(view))) {}

  virtual ~FlutterView() = default;

  // Prevent copying.
  FlutterView(FlutterView const&) = delete;
  FlutterView& operator=(FlutterView const&) = delete;

#ifdef WINUWP
  // Returns the backing CoreApplicationView for the view.
  ABI::Windows::ApplicationModel::Core::CoreApplicationView* GetNativeWindow() {
    return FlutterDesktopViewGetCoreApplicationView(view_);
  }
#else
  // Returns the backing HWND for the view.
  HWND GetNativeWindow() { return FlutterDesktopViewGetHWND(view_); }
#endif

  // Gets the platform task runner.
  FlutterTaskRunner* GetPlatformTaskRunner() {
    return platform_task_runner_.get();
  }

 private:
  // Handle for interacting with the C API's view.
  FlutterDesktopViewRef view_ = nullptr;

  // The task runner for this view.
  std::unique_ptr<FlutterTaskRunner> platform_task_runner_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_H_
