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
                    int repeats) override;

  // |KeyboardHookHandler|
  void TextHook(FlutterWindowsView* window,
                const std::u16string& text) override;

 private:
  std::function<void(const FlutterKeyEvent&)> onEvent_;
  std::map<uint64_t, uint64_t> pressingRecords_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_KEYBOARD_MANAGER_H_
