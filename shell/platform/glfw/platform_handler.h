// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_PLATFORM_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_PLATFORM_HANDLER_H_

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/glfw/public/flutter_glfw.h"
#include "rapidjson/document.h"

namespace flutter {

// Callback for reading the clipboard data.
typedef const char* (*GetClipboardDataCallback)(FlutterDesktopWindowRef);

// Callback for writing data to the clipboard.
typedef void (*SetClipboardDataCallback)(FlutterDesktopWindowRef, const char*);

// Handler for internal system channels.
class PlatformHandler {
 public:
  explicit PlatformHandler(flutter::BinaryMessenger* messenger,
                           FlutterDesktopWindowRef window,
                           GetClipboardDataCallback get_clipboard_callback,
                           SetClipboardDataCallback set_clipboard_callback);

  virtual ~PlatformHandler();

 private:
  // Called when a method is called on |channel_|;
  void HandleMethodCall(
      const flutter::MethodCall<rapidjson::Document>& method_call,
      std::unique_ptr<flutter::MethodResult<rapidjson::Document>> result);

  // The MethodChannel used for communication with the Flutter engine.
  std::unique_ptr<flutter::MethodChannel<rapidjson::Document>> channel_;

  // A reference to the desktop window.
  FlutterDesktopWindowRef window_;

  // Callback function to get the clipboard data from the window toolkit.
  GetClipboardDataCallback get_clipboard_callback_ = NULL;

  // Callback function to set the clipboard data from the window toolkit.
  SetClipboardDataCallback set_clipboard_callback_ = NULL;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_GLFW_PLATFORM_HANDLER_H_
