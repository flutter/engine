// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_TOP_LEVEL_MESSAGE_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_TOP_LEVEL_MESSAGE_HANDLER_H_

#include <cstdint>

#include <Windows.h>

namespace flutter {

class FlutterWindowsEngine;

class WindowsLifecycleManager {
 public:
  WindowsLifecycleManager(FlutterWindowsEngine& engine);
  virtual ~WindowsLifecycleManager();

  virtual void Quit(int64_t exit_code);

  bool WindowProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l, LRESULT* result);

 private:
  bool IsLastWindowOfProcess();

  FlutterWindowsEngine& engine_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_TOP_LEVEL_MESSAGE_HANDLER_H_
