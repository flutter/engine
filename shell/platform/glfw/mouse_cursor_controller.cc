// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/mouse_cursor_controller.h"

#include <cstdint>
#include <iostream>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/standard_method_codec.h"

static constexpr char kSetAsSystemCursorMethod[] = "setAsSystemCursor";
static constexpr char kPlatformConstantKey[] = "platformConstant";

static constexpr char kSetHiddenMethod[] = "setHidden";
static constexpr char kHiddenKey[] = "hidden";

static constexpr char kChannelName[] = "flutter/mousecursor";

static constexpr char kBadArgumentError[] = "Bad Arguments";
static constexpr char kGLFWError[] = "GLFW Error";

namespace flutter {

// Create an object of |GLFWErrorRecorder| to start recording GLFW errors.
// Resources are released as this object is destructed.
// TODO: After upgrading to GLFW 3.3, replace this with |glfwGetError|.
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

  static int code_;

  static const char *description_;

  GLFWerrorfun previous_;
};

int GLFWErrorRecorder::code_ = 0;
const char *GLFWErrorRecorder::description_ = NULL;

MouseCursorController::MouseCursorController(flutter::BinaryMessenger* messenger,
                                             GLFWwindow* window)
    : channel_(std::make_unique<flutter::MethodChannel<EncodableValue>>(
          messenger,
          kChannelName,
          &flutter::StandardMethodCodec::GetInstance())),
      window_(window) {
  channel_->SetMethodCallHandler(
      [this](
          const flutter::MethodCall<EncodableValue>& call,
          std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
        HandleMethodCall(call, std::move(result));
      });
}


MouseCursorController::~MouseCursorController() {
  for (auto cursorObjectIt : systemCursors_) {
    glfwDestroyCursor(cursorObjectIt.second);
  }
}

void MouseCursorController::setAsSystemCursor(
    const EncodableMap& args,
    flutter::MethodResult<EncodableValue>& result) {
  GLFWErrorRecorder errorRecorder;

  const auto platformConstant = args.find(EncodableValue(kPlatformConstantKey));
  if (platformConstant == args.end() || !platformConstant->second.IsInt()) {
    result.Error(kBadArgumentError,
                  "Could not set cursor, invalid argument platformConstant.");
    return;
  }

  GLFWcursor* cursorObject = resolveSystemCursor(platformConstant->second.IntValue());
  if (cursorObject == NULL) {
    result.Error(kGLFWError, "GLFW failed to create the cursor.");
    return;
  }
  if (glfwErrorOccurred(result))
    return;
  glfwSetCursor(window_, cursorObject);
  if (glfwErrorOccurred(result))
    return;
  result.Success();
}

GLFWcursor* MouseCursorController::resolveSystemCursor(int platformConstant) {
  auto cached = systemCursors_.find(platformConstant);
  if (cached != systemCursors_.end()) {
    return cached->second;
  }
  GLFWcursor* cursorObject = glfwCreateStandardCursor(platformConstant);
  systemCursors_[platformConstant] = cursorObject;
  // |cursorObject| might be null, or glfw might has error.
  // Both cases are unhandled.
  return cursorObject;
}

void MouseCursorController::setHidden(
    const EncodableMap& args,
    flutter::MethodResult<EncodableValue>& result) {
  GLFWErrorRecorder errorRecorder;

  const auto hidden = args.find(EncodableValue(kHiddenKey));
  if (hidden == args.end() || !hidden->second.IsBool()) {
    result.Error(kBadArgumentError,
                  "Could not set hidden, invalid argument hidden.");
    return;
  }

  glfwSetInputMode(window_, GLFW_CURSOR,
    hidden->second.BoolValue() ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
  if (glfwErrorOccurred(result))
    return;
  result.Success();
}

bool MouseCursorController::glfwErrorOccurred(flutter::MethodResult<EncodableValue>& result) {
  if (GLFWErrorRecorder::hasError()) {
    result.Error(kGLFWError, GLFWErrorRecorder::description());
    return true;
  }
  return false;
}

void MouseCursorController::HandleMethodCall(
    const flutter::MethodCall<EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
  const std::string& method = method_call.method_name();

  if (!method_call.arguments() || method_call.arguments()->IsNull()) {
    result->Error(kBadArgumentError, "Method invoked without args");
    return;
  }
  const EncodableValue& args = *method_call.arguments();
  if (!args.IsMap()) {
    result->Error(kBadArgumentError,
                  "Could not set cursor, argument is not a map.");
    return;
  }
  const EncodableMap& argMap = args.MapValue();

  if (method.compare(kSetAsSystemCursorMethod) == 0) {
    setAsSystemCursor(argMap, *result);
    return;
  } else if (method.compare(kSetHiddenMethod) == 0) {
    setHidden(argMap, *result);
    return;
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
