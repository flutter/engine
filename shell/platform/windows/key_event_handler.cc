// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/key_event_handler.h"
#include "flutter/shell/platform/windows/keycodes/keyboard_map_windows.h"

#include <chrono>
#include <windows.h>

#include <iostream>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/standard_message_codec.h"

static constexpr char kChannelName[] = "flutter/hardwarekeyevent";

static constexpr char kKeyCodeKey[] = "keyCode";
static constexpr char kKeyMapKey[] = "keymap";
static constexpr char kTypeKey[] = "type";

static constexpr char kAndroidKeyMap[] = "android";
static constexpr char kKeyUp[] = "keyup";
static constexpr char kKeyDown[] = "keydown";

namespace flutter {

KeyEventHandler::KeyEventHandler(flutter::BinaryMessenger* messenger)
    : channel_(
          std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
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
  // TODO: Translate to a cross-platform key code system rather than passing
  // the native key code.
  // rapidjson::Document event(rapidjson::kObjectType);
  // auto& allocator = event.GetAllocator();
  // event.AddMember(kKeyCodeKey, key, allocator);
  // event.AddMember(kKeyMapKey, kAndroidKeyMap, allocator);
  size_t timestamp =
        std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  EncodableMap logicKeyPayload = {
    {EncodableValue(10000), EncodableValue(g_windows_to_logical_key(key))},
    // {EncodableValue(20000), EncodableValue(key)},
  };
  EncodableMap physicalKeyPayload = {
    {EncodableValue(10000000), EncodableValue(g_windows_to_physical_key.at(scancode))},
    // {EncodableValue(20000), EncodableValue(key)},
  };
  EncodableMap payload = {
    {EncodableValue(100), EncodableValue(logicKeyPayload)},
    {EncodableValue(200), EncodableValue(physicalKeyPayload)},
  };

  EncodableMap map = {
    {EncodableValue(1),EncodableValue(static_cast<int>(timestamp))},
    {EncodableValue(2), EncodableValue(action)},
    {EncodableValue(3), EncodableValue(payload)},

  };
  EncodableValue value(map);
  // std::cerr << "Value \n";
  // std::cerr << value << std::endl;
  // switch (action) {
  //   case WM_KEYDOWN:
  //     // event.AddMember(kTypeKey, kKeyDown, allocator);
  //     break;
  //   case WM_KEYUP:
  //     // event.AddMember(kTypeKey, kKeyUp, allocator);
  //     break;
  //   default:
  //     std::cerr << "Unknown key event action: " << action << std::endl;
  //     return;
  // }

  channel_->Send(value);
}

}  // namespace flutter
