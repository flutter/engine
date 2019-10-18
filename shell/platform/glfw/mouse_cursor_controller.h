// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_MOUSE_CURSOR_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_MOUSE_CURSOR_CONTROLLER_H_

#include <map>
#include <memory>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/common/cpp/text_input_model.h"
#include "flutter/shell/platform/glfw/keyboard_hook_handler.h"
#include "flutter/shell/platform/glfw/public/flutter_glfw.h"

namespace flutter {

typedef uint32_t CURSOR;

// TODOC
class MouseCursorController {
 public:
  explicit MouseCursorController(flutter::BinaryMessenger* messenger,
                                 GLFWwindow* window);

  virtual ~MouseCursorController();

 private:
  // Called when a method is called on |channel_|;
  void HandleMethodCall(
      const flutter::MethodCall<rapidjson::Document>& method_call,
      std::unique_ptr<flutter::MethodResult<rapidjson::Document>> result);

  // TODOC
  // Returns true if successful, with |result| unmodified.
  // Returns false if failed, with |result| set to the error.
  bool changeToCursor(
      CURSOR cursor,
      flutter::MethodResult<rapidjson::Document>& result);

  // TODOC
  // May return NULL. May contain unhandled GLFW error.
  // Doesn't handle hidden.
  GLFWcursor* resolveCursor(CURSOR cursor);

  // TODOC
  // Guarantees to return a non-zero int.
  // Doesn't handle hidden.
  int resolveSystemCursorConstant(CURSOR cursor);

  // TODOC
  bool glfwErrorOccurred(flutter::MethodResult<rapidjson::Document>& result);

  // TODOC
  std::map<CURSOR, GLFWcursor*> cursorObjects_;

  // The MethodChannel used for communication with the Flutter engine.
  std::unique_ptr<flutter::MethodChannel<rapidjson::Document>> channel_;

  // TODOC
  CURSOR currentCursor_;

  // TODOC
  GLFWwindow* window_;
};

// TODOC
enum MouseCursors: CURSOR {
  // Constants here must be kept in sync with mouse_cursors.dart

  kMouseCursorNone = 0x334c4a4c,
  kMouseCursorBasic = 0xf17aaabc,
  kMouseCursorClick = 0xa8affc08,
  kMouseCursorText = 0x1cb251ec,
  kMouseCursorNo = 0x7fa3b767,
  kMouseCursorGrab = 0x28b91f80,
  kMouseCursorGrabbing = 0x6631ce3e,
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_GLFW_MOUSE_CURSOR_CONTROLLER_H_
