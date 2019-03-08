// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/cpp/glfw/key_event_handler.h"

#include <json/json.h>
#include <iostream>

#include "flutter/shell/platform/cpp/client_wrapper/include/flutter/json_message_codec.h"

static constexpr char kChannelName[] = "flutter/keyevent";

static constexpr char kKeyCodeKey[] = "keyCode";
static constexpr char kKeyMapKey[] = "keymap";
static constexpr char kTypeKey[] = "type";

static constexpr char kAndroidKeyMap[] = "android";
static constexpr char kKeyUp[] = "keyup";
static constexpr char kKeyDown[] = "keydown";

namespace shell {

KeyEventHandler::KeyEventHandler(flutter::BinaryMessenger* messenger)
    : channel_(std::make_unique<flutter::BasicMessageChannel<Json::Value>>(
          messenger,
          kChannelName,
          &flutter::JsonMessageCodec::GetInstance())) {}

KeyEventHandler::~KeyEventHandler() {}

void KeyEventHandler::CharHook(GLFWwindow* window, unsigned int code_point) {}

void KeyEventHandler::KeyboardHook(GLFWwindow* window,
                                   int key,
                                   int scancode,
                                   int action,
                                   int mods) {
  // TODO: Translate to a cross-platform key code system rather than passing
  // the native key code.
  Json::Value event;
  event[kKeyCodeKey] = key;
  event[kKeyMapKey] = kAndroidKeyMap;

  switch (action) {
    case GLFW_PRESS:
      event[kTypeKey] = kKeyDown;
      break;
    case GLFW_RELEASE:
      event[kTypeKey] = kKeyUp;
      break;
    default:
      std::cerr << "Unknown key event action: " << action << std::endl;
      return;
  }
  channel_->Send(event);
}

}  // namespace shell