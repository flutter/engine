// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/key_event_handler.h"

#include <cstddef>
#include <iostream>
#include <map>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/json_message_codec.h"

static constexpr char kChannelName[] = "flutter/keyevent";

static constexpr char kKeyCodeKey[] = "keyCode";
static constexpr char kKeyMapKey[] = "keymap";
static constexpr char kScanCodeKey[] = "scanCode";
static constexpr char kModifiersKey[] = "modifiers";
static constexpr char kTypeKey[] = "type";
static constexpr char kToolkitKey[] = "toolkit";
static constexpr char kCodePoint[] = "unicodeScalarValuesProduced";

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



std::map<int, int> GetMasksMap() {
  std::map<int, int> masks;
  masks[4] = 0x07;
  masks[3] = 0x0F;
  masks[2] = 0x1F;
  masks[1] = 0xFF;
  return masks;
}

std::map<int, int> GetShiftMap() {
  std::map<int, int> bits;
  bits[1] = 0;
  bits[2] = 6;
  bits[3] = 12;
  bits[4] = 18;
  return bits;
}

int DecodeUTF8(const char* utf8) {
  size_t length = strlen(utf8);
  length = length >= 4 ? length : length;

  int shift = length;
  auto masks = GetMasksMap();
  auto bits = GetShiftMap();
  int complement_mask = 0x3F;
  int result = 0;

  size_t current_byte_index = 0;
  while (current_byte_index < length) {
    int current_byte = utf8[current_byte_index];
    int mask =
        current_byte_index == 0 ? masks[length] : complement_mask;

    current_byte_index++;
    result += (current_byte & mask) << bits[shift--];
  }

  return result;
}
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
  int unicodeInt = DecodeUTF8(keyName);
  event.AddMember(kCodePoint, unicodeInt, allocator);
  }
  // std::string nameString(keyName);
  // int first = (int)nameString.at(0);
  // int total = first;
  // int second = 0;
  // if (nameString.size() > 1) {
  //  second = (int)nameString.at(1);
  //  total = (first << 16) | second;
  // }
  // if (keyName != nullptr) {
  //   event.AddMember(kCodePoint, total, allocator);
  // }



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