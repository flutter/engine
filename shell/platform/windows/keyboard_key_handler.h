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

// Handles key events.
//
// This class detects whether an incoming event is a redispatched one,
// dispatches native events to delegates and collect their responses,
// and redispatches events unhandled by Flutter back to the system.
// See |KeyboardHook| for more information about dispatching.
//
// The exact behavior to handle events are further forwarded into
// delegates. See |KeyboardKeyHandlerDelegate| and its subclasses.
class KeyboardKeyHandler : public KeyboardHandlerBase {
 public:
  // An interface for concrete definition of how to asynchronously handle key
  // events.
  class KeyboardKeyHandlerDelegate {
   public:
    // Defines how to how to asynchronously handle key events.
    //
    // If the delegate will wait for an asynchronous response for this event,
    // |KeyboardHook| should return true, and invoke |callback| with the
    // response (whether the event is handled) later for exactly once.
    //
    // Otherwise, the delegate should return false and *never* invoke
    // |callback|. This is considered not handling the event.
    virtual bool KeyboardHook(int key,
                              int scancode,
                              int action,
                              char32_t character,
                              bool extended,
                              bool was_down,
                              std::function<void(bool)> callback) = 0;

    virtual ~KeyboardKeyHandlerDelegate();
  };

  using RedispatchEvent =
      std::function<UINT(UINT cInputs, LPINPUT pInputs, int cbSize)>;

  // Create a KeyboardKeyHandler and specify where to redispatch events.
  //
  // The |redispatch_event| is typically |SendInput|.
  explicit KeyboardKeyHandler(RedispatchEvent redispatch_event);

  ~KeyboardKeyHandler();

  // Add a delegate that handles events received by |KeyboardHook|.
  void AddDelegate(std::unique_ptr<KeyboardKeyHandlerDelegate> delegate);

  // Handles a key event.
  //
  // Returns whether this handler claims to handle the event, which is true if
  // the event is a native event, or false if the event is a redispatched one.
  //
  // Windows requires a synchronous response of whether a key event should be
  // handled, while the query to Flutter is always asynchronous. This is
  // resolved by "redispatching": the response to the native event is always
  // true. If Flutter later decides not to handle the event, an event is then
  // synthesized, dispatched to system, received again, detected, at which time
  // |KeyboardHook| returns false, then falls back to other keyboard handlers.
  //
  // Received events are further dispatched to all added delegates. If any
  // delegate returns true (handled), the event is considered handled. When
  // all delegates responded, any unhandled events are redispatched via
  // |redispatch_event| and recorded. The next (one) time this exact event is
  // received, |KeyboardHook| will skip it and immediately return false.
  //
  // Inherited from |KeyboardHandlerBase|.
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
  void DoRedispatchEvent(const PendingEvent* pending);
  void ResolvePendingEvent(PendingEvent* pending, bool handled);

  std::vector<std::unique_ptr<KeyboardKeyHandlerDelegate>> delegates_;

  // The queue of key events that have been sent to the framework but have not
  // yet received a response.
  std::deque<std::unique_ptr<PendingEvent>> pending_events_;

  // A function used to redispatch synthesized events.
  RedispatchEvent redispatch_event_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_HANDLER_H_
