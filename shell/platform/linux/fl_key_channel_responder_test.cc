// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_channel_responder.h"

#include "gtest/gtest.h"

#include "flutter/shell/platform/linux/fl_binary_messenger_private.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"

static const char* expected_value = nullptr;
static gboolean expected_handled = FALSE;

static FlValue* echo_response_cb(FlValue* echoed_value) {
  gchar* text = fl_value_to_string(echoed_value);
  EXPECT_STREQ(text, expected_value);
  g_free(text);

  FlValue* value = fl_value_new_map();
  fl_value_set_string_take(value, "handled",
                           fl_value_new_bool(expected_handled));
  return value;
}

static void responder_callback(bool handled, gpointer user_data) {
  EXPECT_EQ(handled, expected_handled);
  g_main_loop_quit(static_cast<GMainLoop*>(user_data));
}

// Test sending a letter "A";
TEST(FlKeyChannelResponderTest, SendKeyEvent) {
  g_autoptr(GMainLoop) loop = g_main_loop_new(nullptr, 0);

  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlBinaryMessenger) messenger = fl_binary_messenger_new(engine);
  FlKeyChannelResponderMock mock{
      .value_converter = echo_response_cb,
      .channel_name = "test/echo",
  };
  g_autoptr(FlKeyResponder) responder =
      FL_KEY_RESPONDER(fl_key_channel_responder_new(messenger, &mock));

  char string[] = "A";
  GdkEventKey key_event = GdkEventKey{
      GDK_KEY_PRESS,                         // event type
      nullptr,                               // window (not needed)
      FALSE,                                 // event was sent explicitly
      12345,                                 // time
      0x0,                                   // modifier state
      GDK_KEY_A,                             // key code
      1,                                     // length of string representation
      reinterpret_cast<gchar*>(&string[0]),  // string representation
      0x04,                                  // scan code
      0,                                     // keyboard group
      0,                                     // is a modifier
  };

  // printf("Test 1 %s\n", expected_value);fflush(stdout);
  fl_key_responder_handle_event(responder, &key_event, responder_callback,
                                loop);
  // printf("Test 2 %s\n", expected_value);fflush(stdout);
  expected_value =
      "{type: keydown, keymap: linux, scanCode: 4, toolkit: gtk, keyCode: 65, "
      "modifiers: 0, unicodeScalarValues: 65}";
  expected_handled = FALSE;

  // Blocks here until echo_response_cb is called.
  g_main_loop_run(loop);

  key_event = GdkEventKey{
      GDK_KEY_RELEASE,                       // event type
      nullptr,                               // window (not needed)
      FALSE,                                 // event was sent explicitly
      23456,                                 // time
      0x0,                                   // modifier state
      GDK_KEY_A,                             // key code
      1,                                     // length of string representation
      reinterpret_cast<gchar*>(&string[0]),  // string representation
      0x04,                                  // scan code
      0,                                     // keyboard group
      0,                                     // is a modifier
  };

  fl_key_responder_handle_event(responder, &key_event, responder_callback,
                                loop);
  expected_value =
      "{type: keyup, keymap: linux, scanCode: 4, toolkit: gtk, keyCode: 65, "
      "modifiers: 0, unicodeScalarValues: 65}";
  expected_handled = FALSE;

  // Blocks here until echo_response_cb is called.
  g_main_loop_run(loop);
}

// void test_lock_event(guint key_code,
//                      const char* down_expected,
//                      const char* up_expected) {
//   g_autoptr(GMainLoop) loop = g_main_loop_new(nullptr, 0);

//   FlBinaryMessenger* messenger = fl_binary_messenger_new(engine);
//   g_autoptr(FlKeyChannelResponder) responder = fl_key_channel_responder_new(
//       messenger, text_input_plugin, echo_response_cb, "test/echo");

//   GdkEventKey key_event = GdkEventKey{
//       GDK_KEY_PRESS,  // event type
//       nullptr,        // window (not needed)
//       FALSE,          // event was sent explicitly
//       12345,          // time
//       0x10,           // modifier state
//       key_code,       // key code
//       1,              // length of string representation
//       nullptr,        // string representation
//       0x04,           // scan code
//       0,              // keyboard group
//       0,              // is a modifier
//   };

//   expected_value = down_expected;
//   expected_handled = FALSE;
//   bool handled = fl_key_responder_handle_event(responder, &key_event, loop);
//   EXPECT_TRUE(handled);

//   // Blocks here until echo_response_cb is called.
//   g_main_loop_run(loop);

//   key_event.type = GDK_KEY_RELEASE;
//   key_event.time++;

//   expected_value = up_expected;
//   expected_handled = FALSE;
//   fl_key_responder_handle_event(responder, &key_event, loop);

//   // Blocks here until echo_response_cb is called.
//   g_main_loop_run(loop);
// }

// // Test sending a "NumLock" keypress.
// TEST(FlKeyChannelResponderTest, SendNumLockKeyEvent) {
//   test_lock_event(GDK_KEY_Num_Lock,
//                   "{type: keydown, keymap: linux, scanCode: 4, toolkit: gtk,
//                   " "keyCode: 65407, modifiers: 16}",
//                   "{type: keyup, keymap: linux, scanCode: 4, toolkit: gtk, "
//                   "keyCode: 65407, modifiers: 0}");
// }

// // Test sending a "CapsLock" keypress.
// TEST(FlKeyChannelResponderTest, SendCapsLockKeyEvent) {
//   test_lock_event(GDK_KEY_Caps_Lock,
//                   "{type: keydown, keymap: linux, scanCode: 4, toolkit: gtk,
//                   " "keyCode: 65509, modifiers: 2}",
//                   "{type: keyup, keymap: linux, scanCode: 4, toolkit: gtk, "
//                   "keyCode: 65509, modifiers: 0}");
// }

// // Test sending a "ShiftLock" keypress.
// TEST(FlKeyChannelResponderTest, SendShiftLockKeyEvent) {
//   test_lock_event(GDK_KEY_Shift_Lock,
//                   "{type: keydown, keymap: linux, scanCode: 4, toolkit: gtk,
//                   " "keyCode: 65510, modifiers: 2}",
//                   "{type: keyup, keymap: linux, scanCode: 4, toolkit: gtk, "
//                   "keyCode: 65510, modifiers: 0}");
// }

// TEST(FlKeyChannelResponderTest, TestKeyEventHandledByFramework) {
//   g_autoptr(GMainLoop) loop = g_main_loop_new(nullptr, 0);

//   FlBinaryMessenger* messenger = fl_binary_messenger_new(engine);
//   g_autoptr(FlKeyChannelResponder) responder = fl_key_channel_responder_new(
//       messenger, text_input_plugin, echo_response_cb,
//       "test/key-event-handled");

//   GdkEventKey key_event = GdkEventKey{
//       GDK_KEY_PRESS,  // event type
//       nullptr,        // window (not needed)
//       FALSE,          // event was sent explicitly
//       12345,          // time
//       0x10,           // modifier state
//       GDK_KEY_A,      // key code
//       1,              // length of string representation
//       nullptr,        // string representation
//       0x04,           // scan code
//       0,              // keyboard group
//       0,              // is a modifier
//   };

//   expected_value = "{handled: true}";
//   expected_handled = TRUE;
//   bool handled = fl_key_responder_handle_event(responder, &key_event, loop);
//   // Should always be true, because the event was delayed.
//   EXPECT_TRUE(handled);

//   // Blocks here until echo_response_cb is called.
//   g_main_loop_run(loop);
// }
