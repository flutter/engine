// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/key_event_handler.h"

#include <windows.h>

#include <chrono>
#include <iostream>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/standard_message_codec.h"
#include "flutter/shell/platform/windows/keycodes/keyboard_map_windows.h"

static constexpr char kChannelName[] = "flutter/hardwarekeyevent";

static constexpr char kKeyCodeKey[] = "keyCode";
static constexpr char kKeyMapKey[] = "keymap";
static constexpr char kTypeKey[] = "type";

static constexpr char kAndroidKeyMap[] = "android";
static constexpr char kKeyUp[] = "keyup";
static constexpr char kKeyDown[] = "keydown";

namespace flutter {

KeyEventHandler::KeyEventHandler(flutter::BinaryMessenger* messenger)
    : channel_(std::make_unique<
               flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger,
          kChannelName,
          &flutter::StandardMessageCodec::GetInstance())) {}

KeyEventHandler::~KeyEventHandler() = default;

void KeyEventHandler::CharHook(Win32FlutterWindow* window,
                               char32_t code_point) {}

void KeyEventHandler::KeyboardHook(Win32FlutterWindow* window,
                                   int key,
                                   int scancode,
                                   int action,
                                   int mods,
                                   char32_t character) {
  std::cerr << "Key code " << key << std::endl;
  std::cerr << "Scancode  " << scancode << std::endl;
  std::cerr << "Action " << action << std::endl;
  std::cerr << "Character " << character << std::endl;

  EncodableMap logicKeyPayload;
  EncodableMap physicalKeyPayload;

  std::map<int, uint64_t>::iterator it;
  it = g_windows_to_logical_key.find(key);
  if (it != g_windows_to_logical_key.end()) {
    logicKeyPayload[EncodableValue(10000)] =
        EncodableValue(static_cast<int>(g_windows_to_logical_key.at(key)));
    std::cerr << "Logical key " << g_windows_to_logical_key.at(key)
              << std::endl;
    if (character > 0) {
      logicKeyPayload[EncodableValue(20000)] =
          EncodableValue(static_cast<int>(character));
    }
  } else {
    std::cerr << "Failed to find logical key " << key << std::endl;
  }

  it = g_windows_to_physical_key.find(scancode);
  if (it != g_windows_to_physical_key.end()) {
    physicalKeyPayload[EncodableValue(10000000)] = EncodableValue(
        static_cast<int>(g_windows_to_physical_key.at(scancode)));
    std::cerr << "Physical key " << g_windows_to_physical_key.at(scancode)
              << std::endl;
    if (character > 0) {
      physicalKeyPayload[EncodableValue(20000)] =
          EncodableValue(static_cast<int>(character));
    }
  } else {
    std::cerr << "Failed to find physical scan code " << scancode << std::endl;
  }

  EncodableMap payload = {
      {EncodableValue(100), EncodableValue(logicKeyPayload)},
      {EncodableValue(200), EncodableValue(physicalKeyPayload)},
  };

  size_t timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();

  EncodableMap map = {
      {EncodableValue(1), EncodableValue(static_cast<int>(timestamp))},
      {EncodableValue(2), EncodableValue(action)},
      {EncodableValue(3), EncodableValue(payload)},
  };

  EncodableValue value(map);
  // channel_->Send(value);
}

}  // namespace flutter
