// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_KEY_EVENT_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_KEY_EVENT_HANDLER_H_

#include <memory>
#include <string>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/keyboard_hook_handler.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"
#include "rapidjson/document.h"

namespace flutter {

class FlutterWindowsView;

// Implements a KeyboardHookHandler
//
// Handles key events and forwards them to the Flutter engine.
class FlutterKeyboardManager : public KeyboardHookHandler {
 public:
  explicit FlutterKeyboardManager(flutter::BinaryMessenger* messenger);

  virtual ~FlutterKeyboardManager();

  // |KeyboardHookHandler|
  void KeyboardHook(FlutterWindowsView* window,
                    int key,
                    int scancode,
                    int action,
                    char32_t character) override;

  // |KeyboardHookHandler|
  void TextHook(FlutterWindowsView* window,
                const std::u16string& text) override;

 private:
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_KEY_EVENT_HANDLER_H_
