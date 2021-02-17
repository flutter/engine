// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/keyboard_key_embedder_handler.h"

#include <string>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/shell/platform/windows/testing/engine_embedder_api_modifier.h"
#include "gtest/gtest.h"

namespace flutter {

namespace {

class TestFlutterKeyEvent : public FlutterKeyEvent {
 public:
  TestFlutterKeyEvent(const FlutterKeyEvent& src,
                      FlutterKeyEventCallback callback,
                      void* user_data)
      : character_str(src.character), callback(callback), user_data(user_data) {
    struct_size = src.struct_size;
    timestamp = src.timestamp;
    type = src.type;
    physical = src.physical;
    logical = src.logical;
    character = character_str.c_str();
    synthesized = src.synthesized;
  }

  FlutterKeyEventCallback callback;
  void* user_data;

 private:
  const std::string character_str;
};

}  // namespace

namespace testing {

namespace {
constexpr uint64_t kScanCodeKeyA = 0x1e;
constexpr uint64_t kScanCodeNumpad1 = 0x4f;
constexpr uint64_t kScanCodeNumLock = 0x45;
constexpr uint64_t kScanCodeControl = 0x1d;
constexpr uint64_t kScanCodeShiftLeft = 0x2a;
constexpr uint64_t kScanCodeShiftRight = 0x36;

constexpr uint64_t kVirtualProcessKey = 0xe5;
constexpr uint64_t kVirtualKeyA = 0x41;
constexpr uint64_t kVirtualNumpad1 = 0x61;
constexpr uint64_t kVirtualNumpadEnd = 0x23;
constexpr uint64_t kVirtualNumLock = 0x90;
constexpr uint64_t kVirtualControl = 0xa2;
constexpr uint64_t kVirtualShiftLeft = 0xa0;
constexpr uint64_t kVirtualShiftRight = 0xa1;

constexpr uint64_t kPhysicalKeyA = 0x00070004;
constexpr uint64_t kPhysicalControlLeft = 0x000700e0;
constexpr uint64_t kPhysicalControlRight = 0x000700e4;
constexpr uint64_t kPhysicalShiftLeft = 0x000700e1;
constexpr uint64_t kPhysicalShiftRight = 0x000700e5;

constexpr uint64_t kLogicalKeyA = 0x00000061;
constexpr uint64_t kLogicalControlLeft = 0x00300000105;
constexpr uint64_t kLogicalControlRight = 0x00400000105;
constexpr uint64_t kLogicalShiftLeft = 0x0030000010d;
constexpr uint64_t kLogicalShiftRight = 0x0040000010d;
}  // namespace

// Test the most basic key events.
//
// Press, hold, and release key A on an US keyboard.
TEST(KeyboardKeyEmbedderHandlerTest, BasicKeyPressingAndHolding) {
  std::vector<TestFlutterKeyEvent> results;
  TestFlutterKeyEvent* event;
  bool last_handled = false;

  std::unique_ptr<KeyboardKeyEmbedderHandler> handler =
      std::make_unique<KeyboardKeyEmbedderHandler>(
          [&results](const FlutterKeyEvent& event,
                     FlutterKeyEventCallback callback, void* user_data) {
            results.emplace_back(event, callback, user_data);
          });

  // Press KeyA.
  handler->KeyboardHook(
      kVirtualKeyA, kScanCodeKeyA, WM_KEYDOWN, 'a', false, false,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);

  results.clear();

  // Hold KeyA.
  handler->KeyboardHook(
      kVirtualKeyA, kScanCodeKeyA, WM_KEYDOWN, 'a', false, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, true);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeRepeat);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  event->callback(false, event->user_data);
  EXPECT_EQ(last_handled, false);

  results.clear();

  // Release KeyA.
  handler->KeyboardHook(
      kVirtualKeyA, kScanCodeKeyA, WM_KEYUP, 0, false, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);
  event->callback(false, event->user_data);
}

// Press numpad 1, toggle NumLock, and release numpad 1 on an US
// keyboard.
//
// This is special because the virtual key for numpad 1 will
// change in this process.
TEST(KeyboardKeyEmbedderHandlerTest, ToggleNumLockDuringNumpadPress) {
  std::vector<TestFlutterKeyEvent> results;
  TestFlutterKeyEvent* event;
  bool last_handled = false;

  std::unique_ptr<KeyboardKeyEmbedderHandler> handler =
      std::make_unique<KeyboardKeyEmbedderHandler>(
          [&results](const FlutterKeyEvent& event,
                     FlutterKeyEventCallback callback, void* user_data) {
            results.emplace_back(event, callback, user_data);
          });

  // Press NumPad1.
  handler->KeyboardHook(
      kVirtualNumpad1, kScanCodeNumpad1, WM_KEYDOWN, 0, false, false,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, 0x00070059);
  EXPECT_EQ(event->logical, 0x00200000031);
  // EXPECT_STREQ(event->character, "1"); // TODO
  EXPECT_EQ(event->synthesized, false);
  results.clear();

  // Press NumLock.
  handler->KeyboardHook(
      kVirtualNumLock, kScanCodeNumLock, WM_KEYDOWN, 0, true, false,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, 0x00070053);
  EXPECT_EQ(event->logical, 0x0000010a);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);
  results.clear();

  // Release NumLock.
  handler->KeyboardHook(
      kVirtualNumLock, kScanCodeNumLock, WM_KEYUP, 0, true, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, 0x00070053);
  EXPECT_EQ(event->logical, 0x0000010a);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);
  results.clear();

  // Release NumPad1. (The logical key is now NumpadEnd)
  handler->KeyboardHook(
      kVirtualNumpadEnd, kScanCodeNumpad1, WM_KEYUP, 0, false, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, 0x00070059);
  EXPECT_EQ(event->logical, 0x00200000031);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);
  results.clear();
}

// Key presses that trigger IME should be ignored by this API (and handled by
// compose API).
TEST(KeyboardKeyEmbedderHandlerTest, ImeEventsAreIgnored) {
  std::vector<TestFlutterKeyEvent> results;
  TestFlutterKeyEvent* event;
  bool last_handled = false;

  std::unique_ptr<KeyboardKeyEmbedderHandler> handler =
      std::make_unique<KeyboardKeyEmbedderHandler>(
          [&results](const FlutterKeyEvent& event,
                     FlutterKeyEventCallback callback, void* user_data) {
            results.emplace_back(event, callback, user_data);
          });

  // Press A in an IME
  last_handled = false;
  handler->KeyboardHook(
      kVirtualProcessKey, kScanCodeKeyA, WM_KEYDOWN, 0, true, false,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, true);

  last_handled = false;
  handler->KeyboardHook(
      // The up event for an IME press has a normal virtual key.
      kVirtualKeyA, kScanCodeKeyA, WM_KEYUP, 0, true, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, true);

  // The entire A press does not yield events.
  EXPECT_EQ(results.size(), 0);

  // Press A out of an IME
  last_handled = false;
  handler->KeyboardHook(
      kVirtualKeyA, kScanCodeKeyA, WM_KEYDOWN, 0, true, false,
      [&last_handled](bool handled) { last_handled = handled; });
  // Not decided yet
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
  results.clear();

  last_handled = false;
  handler->KeyboardHook(
      kVirtualKeyA, kScanCodeKeyA, WM_KEYUP, 0, true, false,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
}

// Test if modifier keys that are told apart by the extended bit
// can be identified.
TEST(KeyboardKeyEmbedderHandlerTest, ModifierKeysByExtendedBit) {
  std::vector<TestFlutterKeyEvent> results;
  TestFlutterKeyEvent* event;
  bool last_handled = false;

  std::unique_ptr<KeyboardKeyEmbedderHandler> handler =
      std::make_unique<KeyboardKeyEmbedderHandler>(
          [&results](const FlutterKeyEvent& event,
                     FlutterKeyEventCallback callback, void* user_data) {
            results.emplace_back(event, callback, user_data);
          });

  // Press Ctrl left.
  last_handled = false;
  handler->KeyboardHook(
      kVirtualControl, kScanCodeControl, WM_KEYDOWN, 0, false, false,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalControlLeft);
  EXPECT_EQ(event->logical, kLogicalControlLeft);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
  results.clear();

  // Press Ctrl right.
  last_handled = false;
  handler->KeyboardHook(
      kVirtualControl, kScanCodeControl, WM_KEYDOWN, 0, true, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalControlRight);
  EXPECT_EQ(event->logical, kLogicalControlRight);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
  results.clear();

  // Release Ctrl left.
  last_handled = false;
  handler->KeyboardHook(
      kVirtualControl, kScanCodeControl, WM_KEYUP, 0, false, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalControlLeft);
  EXPECT_EQ(event->logical, kLogicalControlLeft);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
  results.clear();

  // Release Ctrl right.
  last_handled = false;
  handler->KeyboardHook(
      kVirtualControl, kScanCodeControl, WM_KEYUP, 0, true, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalControlRight);
  EXPECT_EQ(event->logical, kLogicalControlRight);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
  results.clear();
}

// Test if modifier keys that are told apart by the virtual key
// can be identified.
TEST(KeyboardKeyEmbedderHandlerTest, ModifierKeysByVirtualKey) {
  std::vector<TestFlutterKeyEvent> results;
  TestFlutterKeyEvent* event;
  bool last_handled = false;

  std::unique_ptr<KeyboardKeyEmbedderHandler> handler =
      std::make_unique<KeyboardKeyEmbedderHandler>(
          [&results](const FlutterKeyEvent& event,
                     FlutterKeyEventCallback callback, void* user_data) {
            results.emplace_back(event, callback, user_data);
          });

  // Press Shift left.
  last_handled = false;
  handler->KeyboardHook(
      kVirtualShiftLeft, kScanCodeShiftLeft, WM_KEYDOWN, 0, false, false,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
  results.clear();

  // Press Shift right.
  last_handled = false;
  handler->KeyboardHook(
      kVirtualShiftRight, kScanCodeShiftRight, WM_KEYDOWN, 0, false, false,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
  results.clear();

  // Release Shift left.
  last_handled = false;
  handler->KeyboardHook(
      kVirtualShiftLeft, kScanCodeShiftLeft, WM_KEYUP, 0, false, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
  results.clear();

  // Release Shift right.
  last_handled = false;
  handler->KeyboardHook(
      kVirtualShiftRight, kScanCodeShiftRight, WM_KEYUP, 0, false, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);
  results.clear();
}

}  // namespace testing
}  // namespace flutter
