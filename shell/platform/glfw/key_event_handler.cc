// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/key_event_handler.h"

#include <iostream>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/json_message_codec.h"

static constexpr char kChannelName[] = "flutter/keyevent";

static constexpr char kKeyCodeKey[] = "keyCode";
static constexpr char kKeyMapKey[] = "keymap";
static constexpr char kScanCodeKey[] = "scanCode";
static constexpr char kModifiersKey[] = "modifiers";
static constexpr char kTypeKey[] = "type";
static constexpr char kToolkitKey[] = "toolkit";
static constexpr char kCharactersIgnoringModifiersKey[] =
    "charactersIgnoringModifiers";

static constexpr char kLinuxKeyMap[] = "linux";
static constexpr char kGLFWKey[] = "glfw";

static constexpr char kKeyUp[] = "keyup";
static constexpr char kKeyDown[] = "keydown";

namespace flutter {

KeyEventHandler::KeyEventHandler(flutter::BinaryMessenger* messenger)
    : channel_(
          std::make_unique<flutter::BasicMessageChannel<rapidjson::Document>>(
              messenger,
              kChannelName,
              &flutter::JsonMessageCodec::GetInstance())) {}

KeyEventHandler::~KeyEventHandler() = default;

void KeyEventHandler::CharHook(GLFWwindow* window, unsigned int code_point) {}

void KeyEventHandler::KeyboardHook(GLFWwindow* window,
                                   int key,
                                   int scancode,
                                   int action,
                                   int mods) {
  // TODO: Translate to a cross-platform key code system rather than passing
  // the native key code.
  rapidjson::Document event(rapidjson::kObjectType);
  auto& allocator = event.GetAllocator();
  event.AddMember(kKeyCodeKey, key, allocator);
  event.AddMember(kKeyMapKey, kLinuxKeyMap, allocator);
  event.AddMember(kScanCodeKey, scancode, allocator);
  event.AddMember(kModifiersKey, mods, allocator);
  event.AddMember(kToolkitKey, kGLFWKey, allocator);

  // Get the name of the printable key, encoded as UTF-8.
  // There's a known issue with glfwGetKeyName, where users with multiple
  // layouts configured on their machines, will not always return the right
  // value. See: https://github.com/glfw/glfw/issues/1462
  const char* keyName = glfwGetKeyName(key, scancode);
  if (keyName != nullptr) {
    event.AddMember(kCharactersIgnoringModifiersKey, std::string(keyName),
                    allocator);
  }

  switch (action) {
    case GLFW_PRESS:
      event.AddMember(kTypeKey, kKeyDown, allocator);
      break;
    case GLFW_RELEASE:
      event.AddMember(kTypeKey, kKeyUp, allocator);
      break;
    default:
      std::cerr << "Unknown key event action: " << action << std::endl;
      return;
  }
  channel_->Send(event);
}

}  // namespace flutter