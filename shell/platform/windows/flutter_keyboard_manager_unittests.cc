// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/flutter_keyboard_manager.h"

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/shell/platform/windows/testing/engine_embedder_api_modifier.h"
#include "gtest/gtest.h"

namespace flutter {

namespace {

std::unique_ptr<char[]> allocCopyString(const char* src) {
  if (src == nullptr)
    return nullptr;
  size_t len = strlen(src);
  std::unique_ptr<char[]> dst = std::make_unique<char[]>(len + 1);
  memcpy(dst.get(), src, (len + 1) * sizeof(char));
  return dst;
}

class TestFlutterKeyEvent : public FlutterKeyEvent {
 public:
  TestFlutterKeyEvent(const FlutterKeyEvent& src) {
    struct_size = src.struct_size;
    timestamp = src.timestamp;
    kind = src.kind;
    physical = src.physical;
    logical = src.logical;
    character_ptr = allocCopyString(src.character);
    character = character_ptr.get();
    synthesized = src.synthesized;
  }

 private:
  std::unique_ptr<char[]> character_ptr;
};

} // namespace

namespace testing {

namespace {
constexpr uint64_t kPhysicalKeyA = 0x1e;
constexpr uint64_t kPhysicalNumpad1 = 0x4f;
constexpr uint64_t kPhysicalNumLock = 0x145;

constexpr uint64_t kLogicalKeyA = 0x41;
constexpr uint64_t kLogicalNumpad1 = 0x61;
constexpr uint64_t kLogicalNumpadEnd = 0x23;
constexpr uint64_t kLogicalNumLock = 0x90;
}

TEST(FlutterKeyboardManager, BasicKeyPressingAndHolding) {
  std::vector<TestFlutterKeyEvent> results;
  TestFlutterKeyEvent* event;

  std::unique_ptr<FlutterKeyboardManager> manager = std::make_unique<FlutterKeyboardManager>(
    [&results](const FlutterKeyEvent& event) {
      results.emplace_back(event);
    }
  );

  // On a US keyboard:
  // Press Numpad1.
  manager->KeyboardHook(nullptr, kLogicalKeyA, kPhysicalKeyA, WM_KEYDOWN, 'a', false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->kind, kFlutterKeyEventKindDown);
  EXPECT_EQ(event->physical, 0x00070004);
  EXPECT_EQ(event->logical, 0x00000061);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);
  results.clear();

  // Hold KeyA.
  manager->KeyboardHook(nullptr, kLogicalKeyA, kPhysicalKeyA, WM_KEYDOWN, 'a', true);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->kind, kFlutterKeyEventKindRepeat);
  EXPECT_EQ(event->physical, 0x00070004);
  EXPECT_EQ(event->logical, 0x00000061);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);
  results.clear();

  // Release KeyA.
  manager->KeyboardHook(nullptr, kLogicalKeyA, kPhysicalKeyA, WM_KEYUP, 0, true);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->kind, kFlutterKeyEventKindUp);
  EXPECT_EQ(event->physical, 0x00070004);
  EXPECT_EQ(event->logical, 0x00000061);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);

}

TEST(FlutterKeyboardManager, ToggleNumLockDuringNumpadPress) {
  std::vector<TestFlutterKeyEvent> results;
  TestFlutterKeyEvent* event;

  std::unique_ptr<FlutterKeyboardManager> manager = std::make_unique<FlutterKeyboardManager>(
    [&results](const FlutterKeyEvent& event) {
      results.emplace_back(event);
    }
  );

  // On a US keyboard:
  // Press NumPad1.
  manager->KeyboardHook(nullptr, kLogicalNumpad1, kPhysicalNumpad1, WM_KEYDOWN, 0, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->kind, kFlutterKeyEventKindDown);
  EXPECT_EQ(event->physical, 0x00070059);
  EXPECT_EQ(event->logical, 0x00200000031);
  // EXPECT_STREQ(event->character, "1"); // TODO
  EXPECT_EQ(event->synthesized, false);
  results.clear();

  // Press NumLock.
  manager->KeyboardHook(nullptr, kLogicalNumLock, kPhysicalNumLock, WM_KEYDOWN, 0, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->kind, kFlutterKeyEventKindDown);
  EXPECT_EQ(event->physical, 0x00070053);
  EXPECT_EQ(event->logical, 0x0000010a);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);
  results.clear();

  // Release NumLock.
  manager->KeyboardHook(nullptr, kLogicalNumLock, kPhysicalNumLock, WM_KEYUP, 0, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->kind, kFlutterKeyEventKindUp);
  EXPECT_EQ(event->physical, 0x00070053);
  EXPECT_EQ(event->logical, 0x0000010a);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);
  results.clear();

  // Release NumPad1. (The logical key is now NumpadEnd)
  manager->KeyboardHook(nullptr, kLogicalNumpadEnd, kPhysicalNumpad1, WM_KEYUP, 0, false);
  EXPECT_EQ(results.size(), 1);
  event = &results[0];
  EXPECT_EQ(event->kind, kFlutterKeyEventKindUp);
  EXPECT_EQ(event->physical, 0x00070059);
  EXPECT_EQ(event->logical, 0x00200000031);
  EXPECT_STREQ(event->character, "");
  EXPECT_EQ(event->synthesized, false);
  results.clear();

}

}  // namespace testing
}  // namespace flutter
