// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_MOUSE_CURSOR_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_MOUSE_CURSOR_CONTROLLER_H_

#include <map>
#include <memory>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/encodable_value.h"
#include "flutter/shell/platform/common/cpp/text_input_model.h"
#include "flutter/shell/platform/glfw/keyboard_hook_handler.h"
#include "flutter/shell/platform/glfw/public/flutter_glfw.h"

namespace flutter {

// TODOC
class MouseCursorController {
 public:
  explicit MouseCursorController(flutter::BinaryMessenger* messenger,
                                 GLFWwindow* window);

  virtual ~MouseCursorController();

 private:
  // Called when a method is called on |channel_|;
  void HandleMethodCall(
      const flutter::MethodCall<EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<EncodableValue>> result);

  // TODOC
  // Returns true if successful, with |result| unmodified.
  // Returns false if failed, with |result| set to the error.
  bool changeToCursor(
      bool hidden,
      int cursor,
      flutter::MethodResult<EncodableValue>& result);

  // TODOC
  // May return NULL. May contain unhandled GLFW error.
  // Doesn't handle hidden.
  GLFWcursor* resolveSystemCursor(int cursor);

  // TODOC
  bool glfwErrorOccurred(flutter::MethodResult<EncodableValue>& result);

  // TODOC
  std::map<int, GLFWcursor*> cursorObjects_;

  // The MethodChannel used for communication with the Flutter engine.
  std::unique_ptr<flutter::MethodChannel<EncodableValue>> channel_;

  // TODOC
  GLFWwindow* window_;

  // TODOC
  bool currentHidden_;

  // TODOC
  int currentSystemConstant_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_GLFW_MOUSE_CURSOR_CONTROLLER_H_
