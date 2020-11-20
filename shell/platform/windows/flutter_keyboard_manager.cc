// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/key_event_handler.h"

#include <windows.h>

#include <iostream>

#include "flutter/shell/platform/common/cpp/json_message_codec.h"

namespace flutter {

namespace {
}  // namespace

FlutterKeyboardManager::FlutterKeyboardManager(flutter::BinaryMessenger* messenger)
    : channel_(
          std::make_unique<flutter::BasicMessageChannel<rapidjson::Document>>(
              messenger,
              kChannelName,
              &flutter::JsonMessageCodec::GetInstance())) {}

FlutterKeyboardManager::~FlutterKeyboardManager() = default;

void FlutterKeyboardManager::TextHook(FlutterWindowsView* view,
                               const std::u16string& code_point) {}

void FlutterKeyboardManager::KeyboardHook(FlutterWindowsView* view,
                                   int key,
                                   int scancode,
                                   int action,
                                   char32_t character) {
  // TODO: Translate to a cross-platform key code system rather than passing
  // the native key code.
  rapidjson::Document event(rapidjson::kObjectType);
  auto& allocator = event.GetAllocator();
  event.AddMember(kKeyCodeKey, key, allocator);
  event.AddMember(kScanCodeKey, scancode, allocator);
  event.AddMember(kCharacterCodePointKey, character, allocator);
  event.AddMember(kKeyMapKey, kWindowsKeyMap, allocator);
  event.AddMember(kModifiersKey, GetModsForKeyState(), allocator);

  switch (action) {
    case WM_KEYDOWN:
      event.AddMember(kTypeKey, kKeyDown, allocator);
      break;
    case WM_KEYUP:
      event.AddMember(kTypeKey, kKeyUp, allocator);
      break;
    default:
      std::cerr << "Unknown key event action: " << action << std::endl;
      return;
  }
  channel_->Send(event);
}

}  // namespace flutter
