// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOWS_LIFECYCLE_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOWS_LIFECYCLE_MANAGER_H_

#include <Windows.h>

#include <cstdint>
#include <map>
#include <optional>

namespace flutter {

class FlutterWindowsEngine;

/// A manager for lifecycle events of the top-level window.
///
/// Currently handles the following events:
/// WM_CLOSE
class WindowsLifecycleManager {
 public:
  WindowsLifecycleManager(FlutterWindowsEngine* engine);
  virtual ~WindowsLifecycleManager();

  virtual void Quit(std::optional<HWND> window, UINT exit_code);

  bool WindowProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l, LRESULT* result);

 private:
  bool IsLastWindowOfProcess();

  FlutterWindowsEngine* engine_;

  std::map<HWND, int> sent_close_messages_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOWS_LIFECYCLE_MANAGER_H_
