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

// Possible reasons for change of keyboard foucs.
enum class FocusChangeDirection {
  kProgrammatic,  // Un-directed focus change.
  kForward,       // Keyboard focus moves forwards, e.g. TAB-key.
  kBackward       // Keyboard focus moves backwards, e.g. Shift+TAB.
};

// Keeps track of registered platform view types and platform view instances,
// and is responsible for processing and responding to platform view related
// method invokations from the framework.
class PlatformViewManager {
 public:
  PlatformViewManager(TaskRunner* task_runner,
                      BinaryMessenger* binary_messenger);

  virtual ~PlatformViewManager();

  // Add a new platform view instance to be lazily instantiated when it is next
  // composited.
  virtual void QueuePlatformViewCreation(std::string_view type_name,
                                         PlatformViewId id);

  // Create a queued platform view instance.
  void InstantiatePlatformView(PlatformViewId id);

  // The runner-facing API calls this method to register a window type
  // corresponding to a platform view identifier supplied to the widget tree.
  void RegisterPlatformViewType(std::string_view type_name,
                                const FlutterPlatformViewTypeEntry& type);

  // The framework may invoke this method when keyboard focus must be given to
  // the platform view.
  void FocusPlatformView(PlatformViewId id,
                         FocusChangeDirection direction,
                         bool focus);

  // Find the HWND corresponding to a platform view id. Returns null if the id
  // has no associated platform view.
  std::optional<HWND> GetNativeHandleForId(PlatformViewId id) const;

 private:
  std::unique_ptr<MethodChannel<EncodableValue>> channel_;

  std::unordered_map<std::string, FlutterPlatformViewTypeEntry>
      platform_view_types_;

  std::unordered_map<PlatformViewId, HWND> platform_views_;

  std::unordered_map<PlatformViewId, std::function<HWND()>>
      pending_platform_views_;

  // Pointer to the task runner of the associated engine.
  TaskRunner* task_runner_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_MANAGER_H_
