// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/flutter_keyboard_manager.h"

#include <windows.h>
#include <assert.h>

#include <chrono>
#include <iostream>
#include <string>
#include <codecvt>

#include "flutter/shell/platform/windows/string_conversion.h"

namespace flutter {

namespace {
}  // namespace

FlutterKeyboardManager::FlutterKeyboardManager(std::function<void(const FlutterKeyEvent&)> onEvent)
    : onEvent_(onEvent) {}

FlutterKeyboardManager::~FlutterKeyboardManager() = default;

void FlutterKeyboardManager::TextHook(FlutterWindowsView* view,
                               const std::u16string& code_point) {}

static uint64_t scanCodeToPhysicalKey(int scancode) {
  return 1;
}

static uint64_t keyCodeToLogicalKey(char32_t character) {
  return 1;
}

void FlutterKeyboardManager::KeyboardHook(FlutterWindowsView* view,
                                          int key,
                                          int scancode,
                                          int action,
                                          char32_t character,
                                          int repeats) {
  const uint64_t physical_key = scanCodeToPhysicalKey(scancode);
  const uint64_t logical_key = keyCodeToLogicalKey(key);
  assert(action == WM_KEYDOWN || action == WM_KEYUP);
  const bool is_physical_down = action == WM_KEYDOWN;

  auto lastLogicalRecordIter = pressingRecords_.find(physical_key);
  const int lastLogicalRecord = lastLogicalRecordIter == pressingRecords_.end() ?
      0 : lastLogicalRecordIter->second;

  
  FlutterKeyEventKind change;
  if (is_physical_down) {
    change = kFlutterKeyEventKindDown;
    if (lastLogicalRecord != 0) {
      // This physical key is being pressed according to the record.
      if (repeats != 0) {
        // A normal repeated key.
        change = kFlutterKeyEventKindRepeat;
      } else {
        // A non-repeated key has been pressed that has the exact physical key as
        // a currently pressed one, usually indicating multiple keyboards are
        // pressing keys with the same physical key, or the up event was lost
        // during a loss of focus. The down event is ignored.
        return;
      }
    } else {
      // This physical key is not being pressed according to the record. It's a
      // normal down event, whether the system event is a repeat or not.
    }
  } else { // isPhysicalDown is false 
    // Handle key up of normal keys
    if (lastLogicalRecord == 0) {
      // The physical key has been released before. It indicates multiple
      // keyboards pressed keys with the same physical key. Ignore the up event.
      return;
    }

    change = kFlutterKeyEventKindUp;
  }

  int nextLogicalRecord;
  switch (change) {
    case kFlutterKeyEventKindDown:
      assert(lastLogicalRecord == 0);
      nextLogicalRecord = logical_key;
      break;
    case kFlutterKeyEventKindUp:
      assert(lastLogicalRecord != 0);
      nextLogicalRecord = 0;
      break;
    case kFlutterKeyEventKindRepeat:
      assert(lastLogicalRecord != 0);
      nextLogicalRecord = lastLogicalRecord;
      break;
  }
  if (nextLogicalRecord == 0) {
    pressingRecords_.erase(lastLogicalRecordIter);
  } else {
    pressingRecords_[physical_key] = nextLogicalRecord;
  }

  FlutterKeyEvent keyData = {};
  keyData.struct_size = sizeof(FlutterKeyEvent);
  keyData.timestamp = 
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  keyData.kind = change;
  keyData.physical = physical_key;
  keyData.logical = logical_key;
  // TODO: Correctly handle UTF-32
  std::wstring text({static_cast<wchar_t>(character)});
  keyData.character = Utf8FromUtf16(text).c_str();
  keyData.synthesized = false;
  onEvent_(keyData);
}

}  // namespace flutter
