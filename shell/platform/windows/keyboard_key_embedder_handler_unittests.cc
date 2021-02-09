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
constexpr uint64_t kPhysicalKeyA = 0x1e;
constexpr uint64_t kPhysicalNumpad1 = 0x4f;
constexpr uint64_t kPhysicalNumLock = 0x45;

constexpr uint64_t kLogicalKeyA = 0x41;
constexpr uint64_t kLogicalNumpad1 = 0x61;
constexpr uint64_t kLogicalNumpadEnd = 0x23;
constexpr uint64_t kLogicalNumLock = 0x90;
}  // namespace

// Test the most basic key events.
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

  // On a US keyboard:
  // Press KeyA.
  handler->KeyboardHook(
      kLogicalKeyA, kPhysicalKeyA, WM_KEYDOWN, 'a', false, false,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, 0x00070004);
  EXPECT_EQ(event->logical, 0x00000061);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  event->callback(true, event->user_data);
  EXPECT_EQ(last_handled, true);

  results.clear();

  // Hold KeyA.
  handler->KeyboardHook(
      kLogicalKeyA, kPhysicalKeyA, WM_KEYDOWN, 'a', false, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(last_handled, true);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeRepeat);
  EXPECT_EQ(event->physical, 0x00070004);
  EXPECT_EQ(event->logical, 0x00000061);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  event->callback(false, event->user_data);
  EXPECT_EQ(last_handled, false);

  results.clear();

  // Release KeyA.
  handler->KeyboardHook(
      kLogicalKeyA, kPhysicalKeyA, WM_KEYUP, 0, false, true,
      [&last_handled](bool handled) { last_handled = handled; });
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, 0x00070004);
  EXPECT_EQ(event->logical, 0x00000061);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);
  event->callback(false, event->user_data);
}

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

  // On a US keyboard:
  // Press NumPad1.
  handler->KeyboardHook(
      kLogicalNumpad1, kPhysicalNumpad1, WM_KEYDOWN, 0, false, false,
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
      kLogicalNumLock, kPhysicalNumLock, WM_KEYDOWN, 0, true, false,
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
      kLogicalNumLock, kPhysicalNumLock, WM_KEYUP, 0, true, true,
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
      kLogicalNumpadEnd, kPhysicalNumpad1, WM_KEYUP, 0, false, true,
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

}  // namespace testing
}  // namespace flutter
