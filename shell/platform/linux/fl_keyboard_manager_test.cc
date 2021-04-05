// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_manager.h"

#include <cstring>

#include "gtest/gtest.h"

namespace {
typedef void (*CallbackHandler)(FlKeyResponderAsyncCallback callback, gpointer user_data);
}

G_BEGIN_DECLS

#define FL_TYPE_KEYBOARD_CALL_RECORD fl_keyboard_call_record_get_type ()
G_DECLARE_FINAL_TYPE(FlKeyboardCallRecord,
                     fl_keyboard_call_record,
                     FL,
                     KEYBOARD_CALL_RECORD,
                     GObject);

typedef struct _FlKeyMockResponder FlKeyMockResponder;
struct _FlKeyboardCallRecord {
  FlKeyMockResponder* responder;
  GdkEventKey* event;
  FlKeyResponderAsyncCallback callback;
  gpointer user_data;
};

#define FL_TYPE_KEY_MOCK_RESPONDER fl_key_mock_responder_get_type ()
G_DECLARE_FINAL_TYPE(FlKeyMockResponder,
                     fl_key_mock_responder,
                     FL,
                     KEY_MOCK_RESPONDER,
                     GObject);

struct _FlKeyMockResponder {
  GObject parent_instance;

  // A weak pointer for a list of FlKeyboardCallRecord.
  GList* call_records;
  CallbackHandler callback_handler;
  int delegate_id;
};

G_END_DECLS

G_DEFINE_TYPE(FlKeyboardCallRecord, fl_keyboard_call_record, G_TYPE_OBJECT)

static void fl_keyboard_call_record_init(FlKeyboardCallRecord* self) {}

// Dispose method for FlKeyboardCallRecord.
static void fl_keyboard_call_record_dispose(GObject* object) {
  // Redundant, but added so that we don't get a warning about unused function
  // for FL_IS_KEYBOARD_CALL_RECORD.
  g_return_if_fail(FL_IS_KEYBOARD_CALL_RECORD(object));

  FlKeyboardCallRecord* self = FL_KEYBOARD_CALL_RECORD(object);
  g_clear_pointer(&self->event, gdk_event_free);
  G_OBJECT_CLASS(fl_keyboard_call_record_parent_class)->dispose(object);
}

// Class Initialization method for FlKeyboardCallRecord class.
static void fl_keyboard_call_record_class_init(FlKeyboardCallRecordClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_call_record_dispose;
}

static FlKeyboardCallRecord* fl_keyboard_call_record_new(
    FlKeyMockResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data) {
  g_return_val_if_fail(FL_IS_KEY_MOCK_RESPONDER(responder), nullptr);
  g_return_val_if_fail(event != nullptr, nullptr);
  g_return_val_if_fail(callback != nullptr, nullptr);
  g_return_val_if_fail(user_data != nullptr, nullptr);

  FlKeyboardCallRecord* self = FL_KEYBOARD_CALL_RECORD(
      g_object_new(fl_keyboard_call_record_get_type(), nullptr));

  self->responder = responder;
  self->event = event;
  self->callback = callback;
  self->user_data = user_data;

  return self;
}

static void dont_respond(FlKeyResponderAsyncCallback callback, gpointer user_data) {}
// static void respond_true(FlKeyResponderAsyncCallback callback, gpointer user_data) {
//   callback(true, user_data);
// }
// static void respond_false(FlKeyResponderAsyncCallback callback, gpointer user_data) {
//   callback(false, user_data);
// }

static void fl_key_mock_responder_iface_init(
    FlKeyResponderInterface* iface);

G_DEFINE_TYPE_WITH_CODE(
  FlKeyMockResponder,
  fl_key_mock_responder,
  G_TYPE_OBJECT,
  G_IMPLEMENT_INTERFACE(FL_TYPE_KEY_RESPONDER,
                        fl_key_mock_responder_iface_init))

static void fl_key_mock_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data);

static void fl_key_mock_responder_iface_init(
    FlKeyResponderInterface* iface) {
  iface->handle_event = fl_key_mock_responder_handle_event;
}

static void fl_key_mock_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data) {
  FlKeyMockResponder* self = FL_KEY_MOCK_RESPONDER(responder);
  self->call_records = g_list_append(self->call_records,
      fl_keyboard_call_record_new(self, event, callback, user_data));
  self->callback_handler(callback, user_data);
}

static void fl_key_mock_responder_class_init(FlKeyMockResponderClass* klass) {
}

static void fl_key_mock_responder_init(FlKeyMockResponder* self) {
}

static FlKeyMockResponder* fl_key_mock_responder_new(
    GList* call_records,
    int delegate_id,
    CallbackHandler callback_handler = dont_respond) {
  g_return_val_if_fail(callback_handler != nullptr, nullptr);

  FlKeyMockResponder* self = FL_KEY_MOCK_RESPONDER(g_object_new(fl_key_mock_responder_get_type(), nullptr));

  self->call_records = call_records;
  self->callback_handler = callback_handler;
  self->delegate_id = delegate_id;

  return self;
}

static GdkEventKey* key_event_new(
    gboolean is_down,
    guint keyval,
    guint16 hardware_keycode,
    guint state,
    gchar* string,
    guint string_length,
    gboolean is_modifier) {
  GdkEventType type = is_down ? GDK_KEY_PRESS : GDK_KEY_RELEASE;
  GdkEventKey* event = reinterpret_cast<GdkEventKey*>(gdk_event_new(type));
  event->window = nullptr;
  event->send_event = TRUE;
  event->time = 0;
  event->state = state;
  event->keyval = keyval;
  event->length = string_length;
  event->string = string;
  event->hardware_keycode = hardware_keycode;
  event->group = 0;
  event->is_modifier = is_modifier ? 1 : 0;
  return event;
}

namespace {
// A global variable to store redispatched scancode. It is a global variable so
// that it can be used in a function without user_data.
int g_redispatch_keyval = 0;
}

static void store_redispatch_keyval(const GdkEvent* event) {
  g_redispatch_keyval = reinterpret_cast<const GdkEventKey*>(event)->keyval;
}

TEST(FlKeyboardManagerTest, SingleDelegateWithAsyncResponds) {
  g_autolist(FlKeyboardCallRecord) call_records = NULL;
  FlKeyboardCallRecord* record;

  gboolean manager_handled = false;
  g_autoptr(FlKeyboardManager) manager = fl_keyboard_manager_new(
      nullptr, store_redispatch_keyval);
  fl_keyboard_manager_add_responder(manager,
      FL_KEY_RESPONDER(fl_key_mock_responder_new(call_records, 0)));

  /// Test 1: One event that is handled by the framework

  // Dispatch a key event
  char stringA[] = "a";
  manager_handled = fl_keyboard_manager_handle_event(
      manager,
      key_event_new(true, 0x50, 0x70, 0, stringA, 1, false));
  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(g_redispatch_keyval, 0);
  EXPECT_EQ(g_list_length(call_records), 1u);
  record = FL_KEYBOARD_CALL_RECORD(g_list_last(call_records)->data);
  EXPECT_EQ(record->responder->delegate_id, 1);
  EXPECT_EQ(record->event->keyval, 0x50u);
  EXPECT_EQ(record->event->hardware_keycode, 0x70u);

  record->callback(true, record->user_data);
  EXPECT_EQ(g_redispatch_keyval, 0);

  g_redispatch_keyval = 0;
}
