// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/mouse_cursor_controller.h"

#include <cstdint>
#include <iostream>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/standard_method_codec.h"

static constexpr char kSetAsSystemCursorMethod[] = "setAsSystemCursor";
static constexpr char kSystemConstantKey[] = "systemConstant";
static constexpr char kHiddenKey[] = "hidden";

static constexpr char kChannelName[] = "flutter/mousecursor";

static constexpr char kBadArgumentError[] = "Bad Arguments";
static constexpr char kGLFWError[] = "GLFW Error";

static constexpr int kDefaultSystemConstant = GLFW_ARROW_CURSOR;

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
    : channel_(std::make_unique<flutter::MethodChannel<EncodableValue>>(
          messenger,
          kChannelName,
          &flutter::StandardMethodCodec::GetInstance())),
      window_(window),
      currentHidden_(false),
      currentSystemConstant_(kDefaultSystemConstant) {
  channel_->SetMethodCallHandler(
      [this](
          const flutter::MethodCall<EncodableValue>& call,
          std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
        HandleMethodCall(call, std::move(result));
      });
}


MouseCursorController::~MouseCursorController() = default;

bool MouseCursorController::changeToCursor(
    bool hidden,
    int systemConstant,
    flutter::MethodResult<EncodableValue>& result) {
  GLFWErrorRecorder errorRecorder;

  bool successful = [&] {
    if (hidden != currentHidden_) {
      glfwSetInputMode(window_, GLFW_CURSOR,
        hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
      if (glfwErrorOccurred(result))
        return false;
    }
    if (hidden != currentHidden_ || systemConstant != currentSystemConstant_) {
      GLFWcursor* cursorObject = resolveSystemCursor(systemConstant);
      if (cursorObject == NULL) {
        result.Error(kGLFWError, "GLFW failed to create the cursor.");
        return false;
      }
      if (glfwErrorOccurred(result))
        return false;
      glfwSetCursor(window_, cursorObject);
      if (glfwErrorOccurred(result))
        return false;
    }
    return true;
  }();

  currentHidden_ = hidden;
  currentSystemConstant_ = systemConstant;

  return successful;
}

GLFWcursor* MouseCursorController::resolveSystemCursor(int systemConstant) {
  auto cached = cursorObjects_.find(systemConstant);
  if (cached != cursorObjects_.end()) {
    return cached->second;
  }
  GLFWcursor* cursorObject = glfwCreateStandardCursor(systemConstant);
  cursorObjects_[systemConstant] = cursorObject;
  // |cursorObject| might be null, or glfw might has error.
  // Both cases are unhandled.
  return cursorObject;

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

  if (method.compare(kSetAsSystemCursorMethod) == 0) {
    // Argument list is: {
    //  "systemConstant": <uint>,
    //  "hidden": <bool>
    // }
    if (!args.IsMap()) {
      result->Error(kBadArgumentError,
                    "Could not set cursor, argument is not a map.");
      return;
    }
    const EncodableMap& argMap = args.MapValue();
    const auto systemConstant = argMap.find(EncodableValue(kSystemConstantKey));
    const auto hidden = argMap.find(EncodableValue(kHiddenKey));
    if (systemConstant == argMap.end() || !systemConstant->second.IsInt()) {
      result->Error(kBadArgumentError,
                    "Could not set cursor, invalid argument systemConstant.");
      return;
    }
    if (hidden == argMap.end() || !hidden->second.IsBool()) {
      result->Error(kBadArgumentError,
                    "Could not set cursor, invalid argument hidden.");
      return;
    }
    if (!changeToCursor(
      hidden->second.BoolValue(),
      systemConstant->second.IntValue(),
      *result
    )) {
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
