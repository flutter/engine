// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/key_event_handler.h"

#include <iostream>
#include <vector>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/json_message_codec.h"

static constexpr char kChannelName[] = "flutter/keyevent";

static constexpr char kKeyCodeKey[] = "keyCode";
static constexpr char kKeyMapKey[] = "keymap";
static constexpr char kScanCodeKey[] = "scanCode";
static constexpr char kModifiersKey[] = "modifiers";
static constexpr char kTypeKey[] = "type";
static constexpr char kToolkitKey[] = "toolkit";
static constexpr char kUnicodeScalarValuesProduced[] =
    "unicodeScalarValuesProduced";

static constexpr char kLinuxKeyMap[] = "linux";
static constexpr char kGLFWKey[] = "glfw";

static constexpr char kKeyUp[] = "keyup";
static constexpr char kKeyDown[] = "keydown";

// Masks used for UTF-8 to UTF-32 conversion.
static constexpr int kTwoByteMask = 0xC0;
static constexpr int kThreeByteMask = 0xE0;
static constexpr int kFourByteMask = 0xF0;

namespace flutter {

// Information about the UTF-8 encoded code point.
struct UTF8CodePointInfo {
  // The bit-mask that determines the length of the code point.
  int first_byte_mask;
  // The number of bytes of the code point.
  size_t length;
};

// Creates a [UTF8CodePointInfo] from a given byte. [first_byte] must be the first
// byte in the code point.
UTF8CodePointInfo GetUTF8CodePointInfo(int first_byte) {
  UTF8CodePointInfo byte_info;

  // The order matters. Otherwise, it is possible that comparing against i.e.
  // kThreeByteMask and kFourByteMask could be both true.
  if ((first_byte & kFourByteMask) == kFourByteMask) {
    byte_info.first_byte_mask = 0x07;
    byte_info.length = 4;
  } else if ((first_byte & kThreeByteMask) == kThreeByteMask) {
    byte_info.first_byte_mask = 0x0F;
    byte_info.length = 3;
  } else if ((first_byte & kTwoByteMask) == kTwoByteMask) {
    byte_info.first_byte_mask = 0x1F;
    byte_info.length = 2;
  } else {
    byte_info.first_byte_mask = 0xFF;
    byte_info.length = 1;
  }
  return byte_info;
}

// Queries GLFW for the printable key name given a [key] and [scan_code] and
// converts it to UTF-32. The Flutter framework accepts only one 32 bit
// int, therefore, if the [code_point]'s length is > 4 (e.g. has two code
// points), only the first will be used. This unlikely but there is no guarantee
// that it won't happen.
bool GetGLFWCodePoint(int key, int scan_code, uint32_t& code_point) {
  // Get the name of the printable key, encoded as UTF-8.
  // There's a known issue with glfwGetKeyName, where users with multiple
  // layouts configured on their machines, will not always return the right
  // value. See: https://github.com/glfw/glfw/issues/1462
  const char* utf8 = glfwGetKeyName(key, scan_code);
  if (utf8 == nullptr) {
    return false;
  }
  // The first byte determines the length of the whole code point.
  auto byte_info = GetUTF8CodePointInfo(utf8[0]);
  size_t length = byte_info.length;
  // Tracks how many bits the current byte should shift to the left.
  int shift = length - 1;

  // The number of bits to shift a byte depending on its position.
  std::vector<int> bits = {0, 6, 12, 18};

  int complement_mask = 0x3F;
  uint32_t result = 0;

  size_t current_byte_index = 0;
  while (current_byte_index < length) {
    int current_byte = utf8[current_byte_index];
    int mask =
        current_byte_index == 0 ? byte_info.first_byte_mask : complement_mask;
    current_byte_index++;
    result += (current_byte & mask) << bits[shift--];
  }
  code_point = result;

  return true;
}

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

  uint32_t unicodeInt;
  bool result = GetGLFWCodePoint(key, scancode, unicodeInt);
  if (result) {
    event.AddMember(kUnicodeScalarValuesProduced, unicodeInt, allocator);
  }

  switch (action) {
    case GLFW_PRESS:
    case GLFW_REPEAT:
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