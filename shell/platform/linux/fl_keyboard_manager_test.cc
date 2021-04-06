// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_manager.h"

#include <cstring>

#include "gtest/gtest.h"

namespace {
typedef void (*CallbackHandler)(FlKeyResponderAsyncCallback callback,
                                gpointer user_data);
}

G_BEGIN_DECLS

#define FL_TYPE_KEYBOARD_CALL_RECORD fl_keyboard_call_record_get_type()
G_DECLARE_FINAL_TYPE(FlKeyboardCallRecord,
                     fl_keyboard_call_record,
                     FL,
                     KEYBOARD_CALL_RECORD,
                     GObject);

typedef struct _FlKeyMockResponder FlKeyMockResponder;
struct _FlKeyboardCallRecord {
  GObject parent_instance;

  FlKeyMockResponder* responder;
  GdkEventKey* event;
  FlKeyResponderAsyncCallback callback;
  gpointer user_data;
};

#define FL_TYPE_KEY_MOCK_RESPONDER fl_key_mock_responder_get_type()
G_DECLARE_FINAL_TYPE(FlKeyMockResponder,
                     fl_key_mock_responder,
                     FL,
                     KEY_MOCK_RESPONDER,
                     GObject);

struct _FlKeyMockResponder {
  GObject parent_instance;

  // A weak pointer for a list of FlKeyboardCallRecord.
  GPtrArray* call_records;
  CallbackHandler callback_handler;
  int delegate_id;
};

G_END_DECLS

G_DEFINE_TYPE(FlKeyboardCallRecord, fl_keyboard_call_record, G_TYPE_OBJECT)

static void fl_keyboard_call_record_init(FlKeyboardCallRecord* self) {}

// Dispose method for FlKeyboardCallRecord.
static void fl_keyboard_call_record_dispose(GObject* object) {
  g_return_if_fail(FL_IS_KEYBOARD_CALL_RECORD(object));

  FlKeyboardCallRecord* self = FL_KEYBOARD_CALL_RECORD(object);
  g_clear_pointer(&self->event, gdk_event_free);
  G_OBJECT_CLASS(fl_keyboard_call_record_parent_class)->dispose(object);
}

// Class Initialization method for FlKeyboardCallRecord class.
static void fl_keyboard_call_record_class_init(
    FlKeyboardCallRecordClass* klass) {
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

static void dont_respond(FlKeyResponderAsyncCallback callback,
                         gpointer user_data) {}
// static void respond_true(FlKeyResponderAsyncCallback callback, gpointer
// user_data) {
//   callback(true, user_data);
// }
// static void respond_false(FlKeyResponderAsyncCallback callback, gpointer
// user_data) {
//   callback(false, user_data);
// }

static void fl_key_mock_responder_iface_init(FlKeyResponderInterface* iface);

G_DEFINE_TYPE_WITH_CODE(FlKeyMockResponder,
                        fl_key_mock_responder,
                        G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(FL_TYPE_KEY_RESPONDER,
                                              fl_key_mock_responder_iface_init))

static void fl_key_mock_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data);

static void fl_key_mock_responder_iface_init(FlKeyResponderInterface* iface) {
  iface->handle_event = fl_key_mock_responder_handle_event;
}

static void fl_key_mock_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data) {
  printf("MockHandle 1\n");
  FlKeyMockResponder* self = FL_KEY_MOCK_RESPONDER(responder);
  g_ptr_array_add(self->call_records,
                  FL_KEYBOARD_CALL_RECORD(fl_keyboard_call_record_new(
                      self, event, callback, user_data)));
  printf("MockHandle [0] %lx\n", (uint64_t)FL_KEYBOARD_CALL_RECORD(
                                     g_ptr_array_index(self->call_records, 0)));
  self->callback_handler(callback, user_data);
}

static void fl_key_mock_responder_class_init(FlKeyMockResponderClass* klass) {}

static void fl_key_mock_responder_init(FlKeyMockResponder* self) {}

static FlKeyMockResponder* fl_key_mock_responder_new(
    GPtrArray* call_records,
    int delegate_id,
    CallbackHandler callback_handler) {
  g_return_val_if_fail(callback_handler != nullptr, nullptr);

  FlKeyMockResponder* self = FL_KEY_MOCK_RESPONDER(
      g_object_new(fl_key_mock_responder_get_type(), nullptr));

  self->call_records = call_records;
  self->callback_handler = callback_handler;
  self->delegate_id = delegate_id;

  return self;
}

static void g_ptr_array_clear(GPtrArray* array) {
  g_ptr_array_remove_range(array, 0, array->len);
}

namespace {
// A global variable to store new event. It is a global variable so that it can
// be returned as a dynamically allocated object for easy use.
GdkEventKey _g_key_event;
}  // namespace

static GdkEventKey* key_event_new(gboolean is_down,
                                  guint keyval,
                                  guint16 hardware_keycode,
                                  guint state,
                                  gboolean is_modifier) {
  _g_key_event.type = is_down ? GDK_KEY_PRESS : GDK_KEY_RELEASE;
  _g_key_event.window = nullptr;
  _g_key_event.send_event = FALSE;
  _g_key_event.time = 12345;
  _g_key_event.state = state;
  _g_key_event.keyval = keyval;
  _g_key_event.length = 0;
  _g_key_event.string = nullptr;
  _g_key_event.hardware_keycode = hardware_keycode;
  _g_key_event.group = 0;
  _g_key_event.is_modifier = is_modifier ? 1 : 0;
  return &_g_key_event;
}

namespace {
// A global variable to store redispatched scancode. It is a global variable so
// that it can be used in a function without user_data.
int g_redispatch_keyval = 0;
}  // namespace

static void store_redispatch_keyval(const GdkEvent* event) {
  g_redispatch_keyval = reinterpret_cast<const GdkEventKey*>(event)->keyval;
}

TEST(FlKeyboardManagerTest, SingleDelegateWithAsyncResponds) {
  GPtrArray* call_records = g_ptr_array_new_with_free_func(g_object_unref);
  FlKeyboardCallRecord* record;

  gboolean manager_handled = false;
  g_autoptr(FlKeyboardManager) manager =
      fl_keyboard_manager_new(nullptr, store_redispatch_keyval);
  fl_keyboard_manager_add_responder(
      manager, FL_KEY_RESPONDER(
                   fl_key_mock_responder_new(call_records, 1, dont_respond)));

  /// Test 1: One event that is handled by the framework

  // Dispatch a key event
  manager_handled = fl_keyboard_manager_handle_event(
      manager, key_event_new(true, GDK_KEY_a, 0x26, 0x10, false));
  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(g_redispatch_keyval, 0);
  EXPECT_EQ(call_records->len, 1u);
  gpointer record_ptr = g_ptr_array_index(call_records, 0);
  record = FL_KEYBOARD_CALL_RECORD(record_ptr);
  EXPECT_EQ(record->responder->delegate_id, 1);
  EXPECT_EQ(record->event->keyval, 0x61u);
  EXPECT_EQ(record->event->hardware_keycode, 0x26u);

  EXPECT_FALSE(fl_keyboard_manager_has_pending_redispatched(manager));
  record->callback(true, record->user_data);
  EXPECT_EQ(g_redispatch_keyval, 0);
  EXPECT_FALSE(fl_keyboard_manager_has_pending_redispatched(manager));

  g_ptr_array_clear(call_records);

  /// Test 2: Two events that are unhandled by the framework

  g_redispatch_keyval = 0;
}

// #PRESS   keyval 0x61 keycode 0x26 state 0x10 ismod 0 snd 0 grp 0 time 1762702987
// #RELEASE keyval 0x61 keycode 0x26 state 0x10 ismod 0 snd 0 grp 0 time 1762703128
