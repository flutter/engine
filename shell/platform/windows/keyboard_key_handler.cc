// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/keyboard_key_handler.h"

#include <windows.h>

#include <iostream>

#include "flutter/shell/platform/common/json_message_codec.h"

namespace flutter {

namespace {

static constexpr char kChannelName[] = "flutter/keyevent";

static constexpr char kKeyCodeKey[] = "keyCode";
static constexpr char kScanCodeKey[] = "scanCode";
static constexpr char kCharacterCodePointKey[] = "characterCodePoint";
static constexpr char kModifiersKey[] = "modifiers";
static constexpr char kKeyMapKey[] = "keymap";
static constexpr char kTypeKey[] = "type";
static constexpr char kHandledKey[] = "handled";

static constexpr char kWindowsKeyMap[] = "windows";
static constexpr char kKeyUp[] = "keyup";
static constexpr char kKeyDown[] = "keydown";

// The maximum number of pending events to keep before
// emitting a warning on the console about unhandled events.
static constexpr int kMaxPendingEvents = 1000;

// This uses event data instead of generating a serial number because
// information can't be attached to the redispatched events, so it has to be
// possible to compute an ID from the identifying data in the event when it is
// received again in order to differentiate between events that are new, and
// events that have been redispatched.
//
// Another alternative would be to compute a checksum from all the data in the
// event (just compute it over the bytes in the struct, probably skipping
// timestamps), but the fields used below are enough to differentiate them, and
// since Windows does some processing on the events (coming up with virtual key
// codes, setting timestamps, etc.), it's not clear that the redispatched
// events would have the same checksums.
uint64_t CalculateEventId(int scancode, int action, bool extended) {
  // Calculate a key event ID based on the scan code of the key pressed,
  // and the flags we care about.
  return scancode | (((action == WM_KEYUP ? KEYEVENTF_KEYUP : 0x0) |
                      (extended ? KEYEVENTF_EXTENDEDKEY : 0x0))
                     << 16);
}

}  // namespace

KeyboardKeyHandler::KeyboardKeyHandlerDelegate::~KeyboardKeyHandlerDelegate() = default;

KeyboardKeyHandler::KeyboardKeyHandler(SendInputDelegate send_input)
    : send_input_(send_input) {
  assert(send_input != nullptr);
}

KeyboardKeyHandler::~KeyboardKeyHandler() = default;

void KeyboardKeyHandler::TextHook(FlutterWindowsView* view,
                               const std::u16string& code_point) {}

void KeyboardKeyHandler::AddDelegate(
      std::unique_ptr<KeyboardKeyHandlerDelegate> delegate) {
  delegates_.push_back(std::move(delegate));
}

const KeyboardKeyHandler::PendingEvent* KeyboardKeyHandler::FindPendingEvent(uint64_t id) {
  if (pending_events_.empty()) {
    return nullptr;
  }
  for (auto iter = pending_events_.begin(); iter != pending_events_.end();
       ++iter) {
    if ((*iter)->id == id) {
      return iter->get();
    }
  }
  return nullptr;
}

void KeyboardKeyHandler::RemovePendingEvent(uint64_t id) {
  for (auto iter = pending_events_.begin(); iter != pending_events_.end();
       ++iter) {
    if ((*iter)->id == id) {
      pending_events_.erase(iter);
      return;
    }
  }
  std::cerr << "Tried to remove pending event with id " << id
            << ", but the event was not found." << std::endl;
}

void KeyboardKeyHandler::RedispatchEvent(const PendingEvent* pending) {
  KEYBDINPUT ki{
    .wVk = 0,
    .wScan = static_cast<WORD>(pending->scancode),
    .dwFlags = pending->dwFlags,
  };
  INPUT input_event;
  input_event.type = INPUT_KEYBOARD;
  input_event.ki = std::move(ki);
  UINT accepted = send_input_(1, &input_event, sizeof(input_event));
  if (accepted != 1) {
    std::cerr << "Unable to synthesize event for unhandled keyboard event "
                  "with scancode "
              << pending->scancode << " (character " << pending->character << ")" << std::endl;
  }
}

bool KeyboardKeyHandler::KeyboardHook(FlutterWindowsView* view,
                                   int key,
                                   int scancode,
                                   int action,
                                   char32_t character,
                                   bool extended,
                                   bool was_down) {
  const uint64_t id = CalculateEventId(scancode, action, extended);
  if (FindPendingEvent(id) != nullptr) {
    // Don't pass messages that we synthesized to the framework again.
    RemovePendingEvent(id);
    return false;
  }

  if (pending_events_.size() > kMaxPendingEvents) {
    std::cerr
        << "There are " << pending_events_.size()
        << " keyboard events that have not yet received a response from the "
        << "framework. Are responses being sent?" << std::endl;
  }
  PendingEvent pending{
    .id = id,
    .scancode = scancode,
    .character = character,
    .dwFlags = static_cast<DWORD>(KEYEVENTF_SCANCODE |
                                  (extended ? KEYEVENTF_EXTENDEDKEY : 0x0) |
                                  (action == WM_KEYUP ? KEYEVENTF_KEYUP : 0x0)),
    .unreplied = delegates_.size(),
    .any_handled = false,
  };
  pending_events_.push_back(std::make_unique<PendingEvent>(std::move(pending)));

  PendingEvent& pending_event = *pending_events_.back();
  // If delegates_ is empty, we need to respond immediately. But this would
  // never happen in real life since all delegates are added by hardcode,
  // therefore we simply assert.
  assert(delegates_.size() != 0);
  for (const auto& delegate : delegates_) {
    delegate->KeyboardHook(key, scancode, action, character, extended,
        was_down, [pending_event = &pending_event, this](bool handled) {
          pending_event->unreplied -= 1;
          pending_event->any_handled = pending_event->any_handled || handled;
          if (pending_event->unreplied == 0) {
            if (!pending_event->any_handled) {
              RedispatchEvent(pending_event);
            } else {
              RemovePendingEvent(pending_event->id);
            }
          }
        });
  }
  return true;
}

void KeyboardKeyHandler::ComposeBeginHook() {
  // Ignore.
}

void KeyboardKeyHandler::ComposeEndHook() {
  // Ignore.
}

void KeyboardKeyHandler::ComposeChangeHook(const std::u16string& text,
                                        int cursor_pos) {
  // Ignore.
}

}  // namespace flutter
