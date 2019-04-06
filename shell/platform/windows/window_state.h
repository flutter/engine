// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_STATE_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_STATE_H_

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/common/cpp/incoming_message_dispatcher.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/key_event_handler.h"
#include "flutter/shell/platform/windows/keyboard_hook_handler.h"
#include "flutter/shell/platform/windows/platform_handler.h"
#include "flutter/shell/platform/windows/text_input_plugin.h"

struct flutter::FlutterWindow;

// Struct for storing state within an instance of the windows native (HWND or
// CoreWindow) Window.
struct FlutterDesktopWindowControllerState {
  //// The win32 window that owns this state object.
  std::unique_ptr<flutter::FlutterWindow> window;

  // The handle to the Flutter engine instance.
  FlutterEngine engine;

  // The window handle given to API clients.
  std::unique_ptr<FlutterDesktopWindow> window_wrapper;
};

// Opaque reference for the native windows itself. This is separate from the
// controller so that it can be provided to plugins without giving them access
// to all of the controller-based functionality.
struct FlutterDesktopWindow {
  // The GLFW window that (indirectly) owns this state object.
  flutter::FlutterWindow* window;

  // Whether or not to track mouse movements to send kHover events.
  bool hover_tracking_enabled = true;

  // The ratio of pixels per screen coordinate for the window.
  double pixels_per_screen_coordinate = 1.0;

  // Resizing triggers a window refresh, but the resize already updates Flutter.
  // To avoid double messages, the refresh after each resize is skipped.
  bool skip_next_window_refresh = false;
};

// Struct for storing state of a Flutter engine instance.
struct FlutterDesktopEngineState {
  // The handle to the Flutter engine instance.
  FlutterEngine engine;
};

// State associated with the plugin registrar.
struct FlutterDesktopPluginRegistrar {
  // The plugin messenger handle given to API clients.
  std::unique_ptr<FlutterDesktopMessenger> messenger;

  // The handle for the window associated with this registrar.
  FlutterDesktopWindow* window;
};

// State associated with the messenger used to communicate with the engine.
struct FlutterDesktopMessenger {
  // The Flutter engine this messenger sends outgoing messages to.
  FlutterEngine engine;

  // The message dispatcher for handling incoming messages.
  flutter::IncomingMessageDispatcher* dispatcher;
};

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_STATE_H_