// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_STATE_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_STATE_H_

#include "flutter/shell/platform/common/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/common/incoming_message_dispatcher.h"
#include "flutter/shell/platform/embedder/embedder.h"

// Structs backing the opaque references used in the C API.
//
// DO NOT ADD ANY NEW CODE HERE. These are legacy, and are being phased out
// in favor of objects that own and manage the relevant functionality.

namespace flutter {
struct FlutterWindowsEngine;
struct FlutterWindowsView;
}  // namespace flutter

// Wrapper to distinguish the view controller ref from the view ref given out
// in the C API.
struct FlutterDesktopViewControllerState {
  // The engine owned by this view controller, if any.
  // This is only used if the view controller was created
  // using |FlutterDesktopViewControllerCreate| as that takes
  // ownership of the engine. View controllers created using
  // |FlutterDesktopEngineCreateViewController| do not take
  // ownership of the engine and do not set this.
  std::unique_ptr<flutter::FlutterWindowsEngine> engine;

  // The view that backs this state object.
  std::unique_ptr<flutter::FlutterWindowsView> view;
};

// Wrapper to distinguish the plugin registrar ref from the engine ref given out
// in the C API.
struct FlutterDesktopPluginRegistrar {
  // The engine that owns this state object.
  flutter::FlutterWindowsEngine* engine = nullptr;
};

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_STATE_H_
