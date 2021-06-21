// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/keyboard_key_handler.h"

#include <windows.h>

#include <iostream>

#include "flutter/shell/platform/common/json_message_codec.h"

namespace flutter {

namespace {

// The maximum number of pending events to keep before
// emitting a warning on the console about unhandled events.
static constexpr int kMaxPendingEvents = 1000;

// Returns true if this key is a special key that Flutter must not redispatch.
//
// This is a temporary solution to
// https://github.com/flutter/flutter/issues/81674, and forces ShiftRight
// KeyDown event to not be redispatched regardless of the framework's response.
//
// If a ShiftRight KeyDown event is not handled by the framework and is
// redispatched, Win32 will not send its following KeyUp event and keeps
// recording ShiftRight as being pressed.
static bool IsEventThatMustNotRedispatch(int virtual_key, bool was_down) {
#ifdef WINUWP
  return false;
#else
  return virtual_key == VK_RSHIFT && !was_down;
#endif
}
}  // namespace

KeyboardKeyHandler::KeyboardKeyHandlerDelegate::~KeyboardKeyHandlerDelegate() =
    default;

KeyboardKeyHandler::KeyboardKeyHandler(SendKeyMessage send_message, EventRedispatcher redispatch_event)
    : send_message_(send_message), redispatch_event_(redispatch_event), last_sequence_id_(1) {}

KeyboardKeyHandler::~KeyboardKeyHandler() = default;

void KeyboardKeyHandler::TextHook(FlutterWindowsView* view,
                                  const std::u16string& code_point) {}

void KeyboardKeyHandler::AddDelegate(
    std::unique_ptr<KeyboardKeyHandlerDelegate> delegate) {
  delegates_.push_back(std::move(delegate));
}

size_t KeyboardKeyHandler::RedispatchedCount() {
  return pending_redispatches_.size();
}

void KeyboardKeyHandler::RedispatchEvent(std::unique_ptr<PendingEvent> event) {
  // TODO(dkwingsmt) consider adding support for redispatching events for UWP
  // in order to support add-to-app.
  // https://github.com/flutter/flutter/issues/70202
#ifdef WINUWP
  return;
#else
  uint8_t scancode = event->scancode;
  char32_t character = event->character;

  INPUT input_event{
      .type = INPUT_KEYBOARD,
      .ki =
          KEYBDINPUT{
              .wVk = 0,
              .wScan = static_cast<WORD>(event->scancode),
              .dwFlags = static_cast<WORD>(
                  KEYEVENTF_SCANCODE |
                  (event->extended ? KEYEVENTF_EXTENDEDKEY : 0x0) |
                  (event->action == WM_KEYUP ? KEYEVENTF_KEYUP : 0x0)),
          },
  };

  pending_redispatches_.push_back(std::move(event));

  UINT accepted = redispatch_event_(1, &input_event, sizeof(input_event));
  if (accepted != 1) {
    std::cerr << "Unable to synthesize event for unhandled keyboard event "
                 "with scancode "
              << scancode << " (character " << character << ")" << std::endl;
  }
#endif
}

bool KeyboardKeyHandler::KeyboardHook(FlutterWindowsView* view,
                                      int key,
                                      int scancode,
                                      int action,
                                      char32_t character,
                                      bool extended,
                                      bool was_down) {
  std::unique_ptr<PendingEvent> incoming =
      std::make_unique<PendingEvent>(PendingEvent{
          .owner = this,
          .key = static_cast<uint32_t>(key),
          .scancode = static_cast<uint8_t>(scancode),
          .action = static_cast<uint32_t>(action),
          .character = character,
          .extended = extended,
          .was_down = was_down,
      });
  incoming->hash = ComputeEventHash(*incoming);
  // Store the raw pointer as the user_data.
  PendingEvent* raw_incoming = incoming.get();

  if (RemoveRedispatchedEvent(*incoming)) {
    return false;
  }

  KeyMessageBuilder builder {
    .handled = false,
  };
  for (const auto& delegate : delegates_) {
    delegate->KeyboardHook(key, scancode, action, character, extended, was_down, builder);
  }
  incoming->force_handled = builder.handled;

  if (pending_responds_.size() > kMaxPendingEvents) {
    std::cerr
        << "There are " << pending_responds_.size()
        << " keyboard events that have not yet received a response from the "
        << "framework. Are responses being sent?" << std::endl;
  }
  pending_responds_.push_back(std::move(incoming));

  FlutterKeyMessage message {
    .struct_size = sizeof(FlutterKeyMessage),
    .num_events = builder.events.size(),
    .events = builder.events.data(),
    .raw_event_size = builder.raw_event.size(),
    .raw_event = builder.raw_event.data(),
  };
  send_message_(message, HandleEmbedderResult, raw_incoming);

  return true;
}

bool KeyboardKeyHandler::RemoveRedispatchedEvent(const PendingEvent& incoming) {
  for (auto iter = pending_redispatches_.begin();
       iter != pending_redispatches_.end(); ++iter) {
    if ((*iter)->hash == incoming.hash) {
      pending_redispatches_.erase(iter);
      return true;
    }
  }
  return false;
}

void KeyboardKeyHandler::ResolvePendingEvent(PendingEvent* pending,
                                             bool handled_by_response) {
  // Find the pending event
  for (auto iter = pending_responds_.begin(); iter != pending_responds_.end();
       ++iter) {
    if (iter->get() == pending) {
      pending_responds_.erase(iter);
      bool handled = handled_by_response || pending->force_handled;
      if (!handled && !IsEventThatMustNotRedispatch(pending->key, pending->was_down)) {
        RedispatchEvent(std::move(*iter));
      }
      // Return here; |iter| can't do ++ after erase.
      return;
    }
  }
  // The pending event should always be found.
  assert(false);
}

void KeyboardKeyHandler::ComposeBeginHook() {
  // Ignore.
}

void KeyboardKeyHandler::ComposeCommitHook() {
  // Ignore.
}

void KeyboardKeyHandler::ComposeEndHook() {
  // Ignore.
}

void KeyboardKeyHandler::ComposeChangeHook(const std::u16string& text,
                                           int cursor_pos) {
  // Ignore.
}

uint64_t KeyboardKeyHandler::ComputeEventHash(const PendingEvent& event) {
  // Calculate a key event ID based on the scan code of the key pressed,
  // and the flags we care about.
  return event.scancode | (((event.action == WM_KEYUP ? KEYEVENTF_KEYUP : 0x0) |
                            (event.extended ? KEYEVENTF_EXTENDEDKEY : 0x0))
                           << 16);
}

void KeyboardKeyHandler::HandleEmbedderResult(bool result, void* user_data) {
  PendingEvent* pending = reinterpret_cast<PendingEvent*>(user_data);
  KeyboardKeyHandler* owner = pending->owner;
  if (owner != nullptr) {
    owner->ResolvePendingEvent(pending, result);
  }
}

}  // namespace flutter
