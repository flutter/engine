// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_embedder_responder.h"

#include "gtest/gtest.h"

#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/shell/platform/linux/fl_binary_messenger_private.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"

static const char* expected_value = nullptr;
static gboolean expected_handled = FALSE;

namespace {
G_DECLARE_FINAL_TYPE(FlKeyboardCallRecord,
                     fl_keyboard_call_record,
                     FL,
                     KEYBOARD_CALL_RECORD,
                     GObject);

struct _FlKeyboardCallRecord {
  GObject parent_instance;

  FlutterKeyEvent* event;
  FlutterKeyEventCallback callback;
  gpointer user_data;
};

G_DEFINE_TYPE(FlKeyboardCallRecord, fl_keyboard_call_record, G_TYPE_OBJECT)

static void fl_keyboard_call_record_init(FlKeyboardCallRecord* self) {}

// Dispose method for FlKeyboardCallRecord.
static void fl_keyboard_call_record_dispose(GObject* object) {
  g_return_if_fail(FL_IS_KEYBOARD_CALL_RECORD(object));

  FlKeyboardCallRecord* self = FL_KEYBOARD_CALL_RECORD(object);
  if (self->event != nullptr) {
    g_free(const_cast<char*>(self->event->character));
    g_free(self->event);
  }
  G_OBJECT_CLASS(fl_keyboard_call_record_parent_class)->dispose(object);
}

// Class Initialization method for FlKeyboardCallRecord class.
static void fl_keyboard_call_record_class_init(
    FlKeyboardCallRecordClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_call_record_dispose;
}

static FlKeyboardCallRecord* fl_keyboard_call_record_new(
    const FlutterKeyEvent* event,
    FlutterKeyEventCallback callback,
    gpointer user_data) {
  g_return_val_if_fail(event != nullptr, nullptr);

  FlKeyboardCallRecord* self = FL_KEYBOARD_CALL_RECORD(
      g_object_new(fl_keyboard_call_record_get_type(), nullptr));

  FlutterKeyEvent* clone_event = g_new(FlutterKeyEvent, 1);
  *clone_event = *event;
  if (event->character != nullptr) {
    size_t character_length = strlen(event->character);
    char* clone_character = g_new(char, character_length + 1);
    strcpy(clone_character, event->character);
    clone_event->character = clone_character;
  }
  self->callback = callback;
  self->user_data = user_data;

  return self;
}

static void responder_callback(bool handled, gpointer user_data) {
  EXPECT_EQ(handled, expected_handled);
  g_main_loop_quit(static_cast<GMainLoop*>(user_data));
}

// Test sending a letter "A";
TEST(FlKeyEmbedderResponderTest, SendKeyEvent) {
  g_autoptr(GMainLoop) loop = g_main_loop_new(nullptr, 0);
  g_autoptr(GPtrArray) call_records =
      g_ptr_array_new_with_free_func(g_object_unref);

  g_autoptr(FlEngine) engine = make_mock_engine();
  FlutterEngineProcTable* embedder_api = fl_engine_get_embedder_api(engine);

  embedder_api->SendKeyEvent = MOCK_ENGINE_PROC(
      SendKeyEvent, ([&call_records](auto engine, const FlutterKeyEvent* event,
                                     FlutterKeyEventCallback callback,
                                     void* user_data) -> FlutterEngineResult {
        g_ptr_array_add(call_records, fl_keyboard_call_record_new(
                                          event, callback, user_data));

        return kSuccess;
      }));

  g_autoptr(FlKeyResponder) responder =
      FL_KEY_RESPONDER(fl_key_embedder_responder_new(engine));

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

}  // namespace
