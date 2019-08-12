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

namespace flutter {

// Queries GLFW for the printable key name given a [key] and [scan_code] and
// converts it to a 32 bit int.The Flutter framework accepts only one 32 bit
// int, therefore, only up to 4 bytes are accepted. If the code point larger
// than 4 bytes, only the first 4 will be used.
//
// Example:
// 中 (11100100 10111000 10101101) converts to 20013 (1001110 00101101).
// This function only covers a multy-byte point to up to 4 bytes.
// TI achieve the conversion, each byte must be masked based on their poisition.
// For one-byte points, the byte is taken as-is, therefore, the mask is 0xFF
// (not all characters in this range are printable, but the GLFW api filters
// non-printable characters by returning nullptr).
// For multi-byte points, only the first byte uses a special mask. The rest of
// the bytes use a "complement mask": 0x3F.
// For two bytes: 0x1F
// For three bytes: 0x0F
// For four bytes: 0x07
//
// Run through:
// 中 = 11100100 10111000 10101101
// It's a 3 byte point, so the mask for the first byte is 0x0F.
// 11100100 & 0x0F = 0100
// For the remaining bytes:
// 10111000 & 0x3F = 111000
// 10101101 & 0x3F = 101101
// All together: 01001110 00101101 (20013)
bool GetGLFWCodePoint(int key, int scan_code, uint32_t& code_point) {
  // Get the name of the printable key, encoded as UTF-8.
  // There's a known issue with glfwGetKeyName, where users with multiple
  // layouts configured on their machines, will not always return the right
  // value. See: https://github.com/glfw/glfw/issues/1462
  const char* utf8 = glfwGetKeyName(key, scan_code);
  if (utf8 == nullptr) {
    return false;
  }

  size_t length = strlen(utf8);
  // Defend against > 4 byte code points.
  length = length >= 4 ? length : length;

  // Tracks how many bits the current byte should shift to the left.
  int shift = length - 1;

  // All possible masks for a 4 byte utf8 code point.
  std::vector<int> masks = {0xFF, 0x1F, 0x0F, 0x07};

  // The number of bits to shift a byte depending on its position.
  std::vector<int> bits = {0, 6, 12, 18};

  int complement_mask = 0x3F;
  uint32_t result = 0;

  size_t current_byte_index = 0;
  while (current_byte_index < length) {
    int current_byte = utf8[current_byte_index];
    // Get the relevant mask for the first byte. Otherwise, get the complement
    // mask.
    int mask = current_byte_index == 0 ? masks[length - 1] : complement_mask;
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
    std::cout << "Got number: " << unicodeInt << std::endl;
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