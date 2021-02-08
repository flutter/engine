// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_HANDLER_H_

#include <deque>
#include <memory>
#include <string>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/basic_message_channel.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/windows/keyboard_handler_base.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"
#include "rapidjson/document.h"

namespace flutter {

class FlutterWindowsView;

// Implements a KeyboardHandlerBase
//
// Handles key events and forwards them to the Flutter engine.
class KeyboardKeyHandler : public KeyboardHandlerBase {
 public:
  class KeyboardKeyHandlerDelegate {
   public:
    // Returns true if the delegate is waiting for an async response,
    // in which case the `callback` should not be called.
    virtual bool KeyboardHook(int key,
                              int scancode,
                              int action,
                              char32_t character,
                              bool extended,
                              bool was_down,
                              std::function<void(bool)> callback) = 0;

    virtual ~KeyboardKeyHandlerDelegate();
  };

  using SendInputDelegate =
      std::function<UINT(UINT cInputs, LPINPUT pInputs, int cbSize)>;

  explicit KeyboardKeyHandler(SendInputDelegate delegate = SendInput);

  ~KeyboardKeyHandler();

  void AddDelegate(std::unique_ptr<KeyboardKeyHandlerDelegate> delegate);

  // |KeyboardHandlerBase|
  bool KeyboardHook(FlutterWindowsView* window,
                    int key,
                    int scancode,
                    int action,
                    char32_t character,
                    bool extended,
                    bool was_down) override;

  // |KeyboardHandlerBase|
  void TextHook(FlutterWindowsView* window,
                const std::u16string& text) override;

  // |KeyboardHandlerBase|
  void ComposeBeginHook() override;

  // |KeyboardHandlerBase|
  void ComposeEndHook() override;

  // |KeyboardHandlerBase|
  void ComposeChangeHook(const std::u16string& text, int cursor_pos) override;

 private:
  struct PendingEvent {
    uint64_t id;
    int scancode;
    char32_t character;
    DWORD dwFlags;
    size_t unreplied;
    bool any_handled;
  };

  const PendingEvent* FindPendingEvent(uint64_t id);
  void RemovePendingEvent(uint64_t id);
  void AddPendingEvent(uint64_t id, int scancode, int action, bool extended);
  void HandleResponse(bool handled,
                      uint64_t id,
                      int action,
                      bool extended,
                      int scancode,
                      int character);
  void RedispatchEvent(const PendingEvent* pending);
  void ResolvePendingEvent(PendingEvent* pending, bool handled);

  std::vector<std::unique_ptr<KeyboardKeyHandlerDelegate>> delegates_;

  // The queue of key events that have been sent to the framework but have not
  // yet received a response.
  std::deque<std::unique_ptr<PendingEvent>> pending_events_;

  // A function used to dispatch synthesized events. Used in testing to inject a
  // test function to collect events. Defaults to the Windows function
  // SendInput.
  SendInputDelegate send_input_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_HANDLER_H_
