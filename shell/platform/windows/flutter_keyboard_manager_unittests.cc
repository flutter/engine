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

TEST(FlutterKeyboardManager, BasicKeyPressing) {
  std::vector<TestFlutterKeyEvent> results;

  std::unique_ptr<FlutterKeyboardManager> manager = std::make_unique<FlutterKeyboardManager>(
    [&results](const FlutterKeyEvent& event) {
      results.emplace_back(event);
    }
  );

  manager->KeyboardHook(nullptr, 0x1, 0x1, WM_KEYDOWN, 0x1, false);
  manager->KeyboardHook(nullptr, 0x1, 0x1, WM_KEYUP, 0x0, false);

  EXPECT_EQ(results.size(), 2);
}

}  // namespace testing
}  // namespace flutter
