// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_CHANNEL_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_CHANNEL_HANDLER_H_

#include <deque>
#include <memory>
#include <string>

#include "flutter/shell/platform/windows/keyboard_key_handler.h"

namespace flutter {

// A delegate of |KeyboardKeyHandler| that handles events by sending the
// raw information through the method channel.
//
// This class communicates with the RawKeyboard API in the framework.
class KeyboardKeyChannelHandler
    : public KeyboardKeyHandler::KeyboardKeyHandlerDelegate {
 public:
  // Create a |KeyboardKeyChannelHandler|.
  KeyboardKeyChannelHandler();

  ~KeyboardKeyChannelHandler();

  // |KeyboardKeyHandler::KeyboardKeyHandlerDelegate|
  void KeyboardHook(int key,
                    int scancode,
                    int action,
                    char32_t character,
                    bool extended,
                    bool was_down,
                    KeyboardKeyHandler::KeyMessageBuilder& builder) override;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_CHANNEL_HANDLER_H_
