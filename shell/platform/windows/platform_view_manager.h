// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_MANAGER_H_

#include <functional>
#include <map>
#include <memory>
#include <optional>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"
#include "flutter/shell/platform/windows/task_runner.h"

namespace flutter {

enum class FocusChangeDirection {
  kProgrammatic,
  kForward,
  kBackward
};

class PlatformViewManager {
 public:
  PlatformViewManager(TaskRunner* task_runner, BinaryMessenger* binary_messenger);

  virtual ~PlatformViewManager();

  virtual void QueuePlatformViewCreation(std::string_view type_name, int64_t id);

  void InstantiatePlatformView(int64_t id);

  void RegisterPlatformViewType(std::string_view type_name, const FlutterPlatformViewTypeEntry& type);

  void FocusPlatformView(int64_t id, FocusChangeDirection direction, bool focus);

  std::optional<HWND> GetNativeHandleForId(int64_t id) const;
 private:
  std::unique_ptr<MethodChannel<EncodableValue>> channel_;

  std::map<std::string, FlutterPlatformViewTypeEntry> platform_view_types_;

  std::map<int64_t, HWND> platform_views_;

  std::map<int64_t, std::function<HWND()>> pending_platform_views_;

  TaskRunner* task_runner_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_MANAGER_H_
