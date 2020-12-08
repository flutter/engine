// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_KEYBOARD_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_KEYBOARD_MANAGER_H_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/keyboard_hook_handler.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"

namespace flutter {

namespace {
// An arbitrary size for the character cache in bytes.
//
// It should hold a UTF-32 character encoded in UTF-8 as long as the trailing
// '\0'.
constexpr size_t kCharacterCacheSize = 8;
}

class FlutterWindowsView;

// Implements a KeyboardHookHandler
//
// Handles key events and forwards them to the Flutter engine.
class FlutterKeyboardManager : public KeyboardHookHandler {
 public:
  explicit FlutterKeyboardManager(std::function<void(const FlutterKeyEvent&)> onEvent);

  virtual ~FlutterKeyboardManager();

  // |KeyboardHookHandler|
  void KeyboardHook(FlutterWindowsView* window,
                    int key,
                    int scancode,
                    int action,
                    char32_t character,
                    bool wasDown) override;

  // |KeyboardHookHandler|
  void TextHook(FlutterWindowsView* window,
                const std::u16string& text) override;

 private:
  void cacheUtf8String(char32_t ch);

  std::function<void(const FlutterKeyEvent&)> onEvent_;
  std::map<uint64_t, uint64_t> pressingRecords_;
  char character_cache_[kCharacterCacheSize];

  static uint64_t getPhysicalKey(int scancode);
  static uint64_t getLogicalKey(int key, int scancode);

  static std::map<uint64_t, uint64_t> windowsToPhysicalMap_;
  static std::map<uint64_t, uint64_t> windowsToLogicalMap_;
  static std::map<uint64_t, uint64_t> scanCodeToLogicalMap_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_KEYBOARD_MANAGER_H_
