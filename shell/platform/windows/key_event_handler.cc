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
                                   int mods) {
  size_t timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();

  std::map<int, uint64_t>::iterator it;
  it = g_windows_to_logical_key.find(key);
  EncodableMap logicKeyPayload;
  EncodableMap physicalKeyPayload;
  if (it != g_windows_to_logical_key.end()) {
    logicKeyPayload = {
        {EncodableValue(10000),
         EncodableValue(static_cast<int>(g_windows_to_logical_key.at(key)))},
        // {EncodableValue(20000), EncodableValue(key)},
    };
  } else {
    std::cerr << "Failed to find logical key " << key << std::endl;
  }
 std::map<int, uint64_t>::iterator itp;
  itp = g_windows_to_physical_key.find(key);
  if (itp != g_windows_to_physical_key.end()) {
    physicalKeyPayload = {
        {EncodableValue(10000000),
         EncodableValue(static_cast<int>(g_windows_to_physical_key.at(key)))},
        // {EncodableValue(20000), EncodableValue(key)},
    };
  } else {
    std::cerr << "Failed to find physical key " << key << std::endl;
  }

  EncodableMap payload = {
      {EncodableValue(100), EncodableValue(logicKeyPayload)},
      {EncodableValue(200), EncodableValue(physicalKeyPayload)},
  };

  EncodableMap map = {
      {EncodableValue(1), EncodableValue(static_cast<int>(timestamp))},
      {EncodableValue(2), EncodableValue(action)},
      {EncodableValue(3), EncodableValue(payload)},
  };
  EncodableValue value(map);

  // channel_->Send(value);
}

}  // namespace flutter
