// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_PLUGIN_H_

#include "flutter/shell/platform/windows/platform_view_manager.h"

#include <functional>
#include <map>
#include <optional>

namespace flutter {

class PlatformViewPlugin : public PlatformViewManager {
 public:
  PlatformViewPlugin(BinaryMessenger* messenger, TaskRunner* task_runner);

  ~PlatformViewPlugin();

  // Find the HWND corresponding to a platform view id. Returns null if the id
  // has no associated platform view.
  std::optional<HWND> GetNativeHandleForId(PlatformViewId id) const;

  // Create a queued platform view instance.
  void InstantiatePlatformView(PlatformViewId id);

  // The runner-facing API calls this method to register a window type
  // corresponding to a platform view identifier supplied to the widget tree.
  void RegisterPlatformViewType(std::string_view type_name,
                                const FlutterPlatformViewTypeEntry& type);

  // | PlatformViewManager |
  bool AddPlatformView(PlatformViewId id, std::string_view type_name) override;

  // | PlatformViewManager |
  bool FocusPlatformView(PlatformViewId id, FocusChangeDirection direction, bool focus) override;

 private:
  std::unordered_map<std::string, FlutterPlatformViewTypeEntry>
      platform_view_types_;

  std::unordered_map<PlatformViewId, HWND> platform_views_;

  std::unordered_map<PlatformViewId, std::function<HWND()>>
      pending_platform_views_;

  // Pointer to the task runner of the associated engine.
  TaskRunner* task_runner_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_PLUGIN_H_
