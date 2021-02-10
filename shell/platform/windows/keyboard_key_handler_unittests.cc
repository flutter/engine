// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "flutter/shell/platform/windows/keyboard_key_handler.h"

#include <rapidjson/document.h>
#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

namespace {

static constexpr int kHandledScanCode = 20;
static constexpr int kHandledScanCode2 = 22;
static constexpr int kUnhandledScanCode = 21;

// A testing |KeyHandlerDelegate| that records all calls
// to |KeyboardHook| and can be customized with whether
// the hook is handled in async.
class MockKeyHandlerDelegate
    : public KeyboardKeyHandler::KeyboardKeyHandlerDelegate {
 public:
  using GetIsAsync = std::function<bool (void)>;

  class KeyboardHookCall {
   public:
    int delegate_id;
    int key;
    int scancode;
    int action;
    char32_t character;
    bool extended;
    bool was_down;
    std::function<void(bool)> callback;
  };

  // Create a |MockKeyHandlerDelegate|.
  //
  // The |delegate_id| is an arbitrary ID to tell between delegates
  // that will be recorded in |KeyboardHookCall|.
  //
  // The |hook_history| will store every call to |KeyboardHookCall| that are
  // handled asynchronously.
  //
  // The |is_async| is a function that the class calls upon every
  // |KeyboardHookCall| to decide whether the call is handled asynchronously.
  // Defaults to always returning true (async).
  MockKeyHandlerDelegate(int delegate_id,
                         std::list<KeyboardHookCall>* hook_history,
                         GetIsAsync is_async = [] { return true; })
      : delegate_id(delegate_id), hook_history(hook_history),
        get_is_async(get_is_async) {}
  virtual ~MockKeyHandlerDelegate() = default;

  virtual bool KeyboardHook(int key,
                            int scancode,
                            int action,
                            char32_t character,
                            bool extended,
                            bool was_down,
                            std::function<void(bool)> callback) {
    if (!is_async()) {
      return false;
    }
    hook_history->push_back(KeyboardHookCall{
        .delegate_id = delegate_id,
        .key = key,
        .scancode = scancode,
        .character = character,
        .extended = extended,
        .was_down = was_down,
        .callback = std::move(callback),
    });
    return true;
  }

  GetIsAsync is_async;
  int delegate_id;
  std::list<KeyboardHookCall>* hook_history;
};

}  // namespace

TEST(KeyboardKeyHandlerTest, BehavesCorrectlyWithSingleDelegate) {
  std::list<MockKeyHandlerDelegate::KeyboardHookCall> hook_history;

  // Capture the scancode of the last redispatched event
  int redispatch_scancode = 0;
  bool delegate_handled = false;
  KeyboardKeyHandler handler([&redispatch_scancode](UINT cInputs,
                                                    LPINPUT pInputs,
                                                    int cbSize) -> UINT {
    EXPECT_TRUE(cbSize > 0);
    redispatch_scancode = pInputs->ki.wScan;
    return 1;
  });
  // Add one delegate
  handler.AddDelegate(
      std::make_unique<MockKeyHandlerDelegate>(1, &hook_history));

  /// Test 1: One event that is handled by the framework

  // Dispatch a key event
  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, false);
  EXPECT_EQ(delegate_handled, true);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 1);
  EXPECT_EQ(hook_history.back().delegate_id, 1);
  EXPECT_EQ(hook_history.back().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.back().was_down, false);

  hook_history.back().callback(true);
  EXPECT_EQ(redispatch_scancode, 0);

  hook_history.clear();

  /// Test 2: Two events that are unhandled by the framework

  // Dispatch a key event.
  // Also this is the same event as the previous one, to test that handled
  // events are cleared from the pending list.
  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, false);
  EXPECT_EQ(delegate_handled, true);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 1);
  EXPECT_EQ(hook_history.back().delegate_id, 1);
  EXPECT_EQ(hook_history.back().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.back().was_down, false);

  // Dispatch another key event
  delegate_handled = handler.KeyboardHook(nullptr, 65, kHandledScanCode2,
                                          WM_KEYUP, L'b', false, true);
  EXPECT_EQ(delegate_handled, true);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 2);
  EXPECT_EQ(hook_history.back().delegate_id, 1);
  EXPECT_EQ(hook_history.back().scancode, kHandledScanCode2);
  EXPECT_EQ(hook_history.back().was_down, true);

  // Resolve the second event first to test out-of-order response
  hook_history.back().callback(false);
  EXPECT_EQ(redispatch_scancode, kHandledScanCode2);

  // Resolve the first event then
  hook_history.front().callback(false);
  EXPECT_EQ(redispatch_scancode, kHandledScanCode);

  EXPECT_EQ(handler.KeyboardHook(nullptr, 64, kHandledScanCode, WM_KEYDOWN,
                                 L'a', false, false),
            false);
  EXPECT_EQ(handler.KeyboardHook(nullptr, 65, kHandledScanCode2, WM_KEYUP, L'b',
                                 false, false),
            false);

  hook_history.clear();
  redispatch_scancode = 0;
}

TEST(KeyboardKeyHandlerTest, BehavesCorrectlyWithTwoAsyncDelegates) {
  std::list<MockKeyHandlerDelegate::KeyboardHookCall> hook_history;

  // Capture the scancode of the last redispatched event
  int redispatch_scancode = 0;
  bool delegate_handled = false;
  KeyboardKeyHandler handler([&redispatch_scancode](UINT cInputs,
                                                    LPINPUT pInputs,
                                                    int cbSize) -> UINT {
    EXPECT_TRUE(cbSize > 0);
    redispatch_scancode = pInputs->ki.wScan;
    return 1;
  });
  // Only add one delegate for now.
  handler.AddDelegate(
      std::make_unique<MockKeyHandlerDelegate>(1, &hook_history));

  /// Test 1: Add a delegate before an event is responded

  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, false);
  EXPECT_EQ(delegate_handled, true);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 1);
  EXPECT_EQ(hook_history.back().delegate_id, 1);
  EXPECT_EQ(hook_history.back().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.back().was_down, false);

  handler.AddDelegate(
      std::make_unique<MockKeyHandlerDelegate>(2, &hook_history));

  // Only one reply is needed because the 2nd delegate is added late.
  hook_history.back().callback(false);
  EXPECT_EQ(redispatch_scancode, kHandledScanCode);

  EXPECT_EQ(handler.KeyboardHook(nullptr, 64, kHandledScanCode, WM_KEYDOWN,
                                 L'a', false, false),
            false);

  redispatch_scancode = 0;
  hook_history.clear();

  /// Test 2: A delegate responds true

  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, false);
  EXPECT_EQ(delegate_handled, true);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 2);
  EXPECT_EQ(hook_history.front().delegate_id, 1);
  EXPECT_EQ(hook_history.front().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.front().was_down, false);
  EXPECT_EQ(hook_history.back().delegate_id, 2);
  EXPECT_EQ(hook_history.back().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.back().was_down, false);

  hook_history.back().callback(true);
  EXPECT_EQ(redispatch_scancode, 0);

  hook_history.front().callback(false);
  EXPECT_EQ(redispatch_scancode, 0);

  redispatch_scancode = 0;
  hook_history.clear();

  /// Test 3: All delegates respond false

  // Also this is the same event as the previous one, to test that handled
  // events are cleared from the pending list.
  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, false);
  EXPECT_EQ(delegate_handled, true);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 2);
  EXPECT_EQ(hook_history.front().delegate_id, 1);
  EXPECT_EQ(hook_history.front().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.front().was_down, false);
  EXPECT_EQ(hook_history.back().delegate_id, 2);
  EXPECT_EQ(hook_history.back().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.back().was_down, false);

  hook_history.front().callback(false);
  EXPECT_EQ(redispatch_scancode, 0);

  hook_history.back().callback(false);
  EXPECT_EQ(redispatch_scancode, kHandledScanCode);

  EXPECT_EQ(handler.KeyboardHook(nullptr, 64, kHandledScanCode, WM_KEYDOWN,
                                 L'a', false, false),
            false);
}

TEST(KeyboardKeyHandlerTest, BehavesCorrectlyWithAsyncAndSyncDelegates) {
  std::list<MockKeyHandlerDelegate::KeyboardHookCall> hook_history;

  // Capture the scancode of the last redispatched event
  int redispatch_scancode = 0;
  bool delegate_handled = false;
  KeyboardKeyHandler handler([&redispatch_scancode](UINT cInputs,
                                                    LPINPUT pInputs,
                                                    int cbSize) -> UINT {
    EXPECT_TRUE(cbSize > 0);
    redispatch_scancode = pInputs->ki.wScan;
    return 1;
  });
  // Add one delegate
  bool delegate1_is_async = true;
  auto delegate1 = std::make_unique<MockKeyHandlerDelegate>(1, &hook_history,
      [&delegate1_is_async]() { return delegate1_is_async; });
  handler.AddDelegate(std::move(delegate1));

  /// Test 1: The only delegate is sync.

  delegate1_is_async = false;
  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, false);
  EXPECT_EQ(delegate_handled, false);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 0);

  // Add another delegate
  bool delegate2_is_async = true;
  auto delegate2 = std::make_unique<MockKeyHandlerDelegate>(2, &hook_history,
      [&delegate2_is_async]() { return delegate2_is_async; });
  handler.AddDelegate(std::move(delegate2));

  redispatch_scancode = 0;
  hook_history.clear();

  /// Test 2: Both delegates are sync

  // Also this is the same event as the previous one, to test that handled
  // events are cleared from the pending list.

  delegate1_is_async = false;
  delegate2_is_async = false;
  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, false);
  EXPECT_EQ(delegate_handled, false);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 0);

  redispatch_scancode = 0;
  hook_history.clear();

  /// Test 3: Only one delegate is sync, the other responds false

  // Also this is the same event as the previous one, to test that handled
  // events are cleared from the pending list.
  delegate1_is_async = true;
  delegate2_is_async = false;
  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, false);
  EXPECT_EQ(delegate_handled, true);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 1);
  EXPECT_EQ(hook_history.front().delegate_id, 1);
  EXPECT_EQ(hook_history.front().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.front().was_down, false);

  hook_history.front().callback(false);
  EXPECT_EQ(redispatch_scancode, kHandledScanCode);

  EXPECT_EQ(handler.KeyboardHook(nullptr, 64, kHandledScanCode, WM_KEYDOWN,
                                 L'a', false, false),
            false);
  redispatch_scancode = 0;
  hook_history.clear();

  /// Test 4: Only one delegate is sync, the other responds true

  // Also this is the same event as the previous one, to test that handled
  // events are cleared from the pending list.
  delegate1_is_async = true;
  delegate2_is_async = false;
  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, false);
  EXPECT_EQ(delegate_handled, true);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 1);
  EXPECT_EQ(hook_history.front().delegate_id, 1);
  EXPECT_EQ(hook_history.front().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.front().was_down, false);

  hook_history.front().callback(true);
  EXPECT_EQ(redispatch_scancode, 0);
}

}  // namespace testing
}  // namespace flutter
