// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/mouse_cursor_controller.h"

#include <cstdint>
#include <iostream>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/json_method_codec.h"

static constexpr char kSetCursorMethod[] = "TextInput.setCursor";

static constexpr char kChannelName[] = "flutter/mousecursor";

static constexpr char kBadArgumentError[] = "Bad Arguments";
static constexpr char kGLFWError[] = "GLFW Error";

typedef bool (*BoolCallback)();
typedef void (*VoidCallback)();

namespace flutter {

// TODOC
class GLFWErrorRecorder {
 public:
  GLFWErrorRecorder() {
    code_ = 0;
    previous_ = glfwSetErrorCallback(glfwErrorHandler);
  }
  ~GLFWErrorRecorder() {
    glfwSetErrorCallback(previous_);
  }

  static bool hasError() {
    return GLFWErrorRecorder::code_ != 0;
  }

  static int code() {
    return GLFWErrorRecorder::code_;
  }

  static const char *description() {
    return GLFWErrorRecorder::description_;
  }

 private:
  static void glfwErrorHandler(int code, const char *description) {
    GLFWErrorRecorder::code_ = code;
    GLFWErrorRecorder::description_ = description;
  }

  // TODOC
  static int code_;

  // TODOC
  static const char *description_;

  GLFWerrorfun previous_;
};

int GLFWErrorRecorder::code_ = 0;
const char *GLFWErrorRecorder::description_ = NULL;

MouseCursorController::MouseCursorController(flutter::BinaryMessenger* messenger,
                                             GLFWwindow* window)
    : channel_(std::make_unique<flutter::MethodChannel<rapidjson::Document>>(
          messenger,
          kChannelName,
          &flutter::JsonMethodCodec::GetInstance())),
      currentCursor_(kMouseCursorBasic),
      window_(window) {
  channel_->SetMethodCallHandler(
      [this](
          const flutter::MethodCall<rapidjson::Document>& call,
          std::unique_ptr<flutter::MethodResult<rapidjson::Document>> result) {
        HandleMethodCall(call, std::move(result));
      });
}


MouseCursorController::~MouseCursorController() = default;

bool MouseCursorController::changeToCursor(
    CURSOR cursor,
    flutter::MethodResult<rapidjson::Document>& result) {
  GLFWErrorRecorder errorRecorder;
  if (cursor == currentCursor_)
    return true;
  currentCursor_ = cursor;
  if (cursor == kMouseCursorNone) {
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    return !glfwErrorOccurred(result);
  }
  if (currentCursor_ == kMouseCursorBasic) {
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwErrorOccurred(result))
      return false;
  }
  GLFWcursor* cursorObject = resolveCursor(cursor);
  if (cursorObject == NULL) {
    result.Error(kGLFWError, "GLFW failed to create the cursor.");
    return false;
  }
  if (glfwErrorOccurred(result))
    return false;
  glfwSetCursor(window_, cursorObject);
  if (glfwErrorOccurred(result))
    return false;

  return true;
}

GLFWcursor* MouseCursorController::resolveCursor(CURSOR cursor) {
  auto cached = cursorObjects_.find(cursor);
  if (cached != cursorObjects_.end()) {
    return cached->second;
  }
  GLFWcursor* cursorObject = glfwCreateStandardCursor(
    resolveSystemCursorConstant(cursor)
  );
  cursorObjects_[cursor] = cursorObject;
  // |cursorObject| might be null, or glfw might has error.
  // Both cases are unhandled.
  return cursorObject;

}

int MouseCursorController::resolveSystemCursorConstant(CURSOR cursor) {
  switch (cursor) {
    case kMouseCursorNone:
      break;
    case kMouseCursorBasic:
      break;
    case kMouseCursorClick:
      return GLFW_HAND_CURSOR;
    case kMouseCursorText:
      return GLFW_IBEAM_CURSOR;
    case kMouseCursorNo:
      break;
    case kMouseCursorGrab:
      return GLFW_HAND_CURSOR;
    case kMouseCursorGrabbing:
      return GLFW_HAND_CURSOR;
  }
  return GLFW_ARROW_CURSOR;
}

bool MouseCursorController::glfwErrorOccurred(flutter::MethodResult<rapidjson::Document>& result) {
  if (GLFWErrorRecorder::hasError()) {
    result.Error(kGLFWError, GLFWErrorRecorder::description());
    return true;
  }
  return false;
}

void MouseCursorController::HandleMethodCall(
    const flutter::MethodCall<rapidjson::Document>& method_call,
    std::unique_ptr<flutter::MethodResult<rapidjson::Document>> result) {
  const std::string& method = method_call.method_name();

  if (!method_call.arguments() || method_call.arguments()->IsNull()) {
    result->Error(kBadArgumentError, "Method invoked without args");
    return;
  }
  const rapidjson::Document& args = *method_call.arguments();

  if (method.compare(kSetCursorMethod) == 0) {
    // Argument list is [device, cursor]. Since GLFW only supports one pointer
    // device, we can ignore argument 0.
    const rapidjson::Value& cursor_json = args[1];
    if (cursor_json.IsNull() || !cursor_json.IsUint()) {
      result->Error(kBadArgumentError,
                    "Could not set cursor, invalid argument cursor.");
      return;
    }
    CURSOR cursor = cursor_json.GetUint();
    if (!changeToCursor(cursor, *result)) {
      return;
    }
  } else {
    // Unhandled method.
    result->NotImplemented();
    return;
  }

  // All error conditions return early, so if nothing has gone wrong indicate
  // success.
  result->Success();
}

}  // namespace flutter
