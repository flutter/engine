// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_manager.h"

#include <cstring>

#include "flutter/shell/platform/linux/testing/mock_text_input_plugin.h"
#include "gtest/gtest.h"

#define FL_KEY_EVENT(target) reinterpret_cast<FlKeyEvent*>(target)

namespace {
typedef void (*CallbackHandler)(FlKeyResponderAsyncCallback callback,
                                gpointer user_data);

constexpr guint16 kKeyCodeKeyA = 0x26u;
constexpr guint16 kKeyCodeKeyB = 0x38u;

G_DECLARE_FINAL_TYPE(FlKeyboardCallRecord,
                     fl_keyboard_call_record,
                     FL,
                     KEYBOARD_CALL_RECORD,
                     GObject);

typedef struct _FlKeyMockResponder FlKeyMockResponder;
struct _FlKeyboardCallRecord {
  GObject parent_instance;

  FlKeyMockResponder* responder;
  FlKeyEvent* event;
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

/***** FlKeyboardCallRecord *****/

G_DEFINE_TYPE(FlKeyboardCallRecord, fl_keyboard_call_record, G_TYPE_OBJECT)

static void fl_keyboard_call_record_init(FlKeyboardCallRecord* self) {}

// Dispose method for FlKeyboardCallRecord.
static void fl_keyboard_call_record_dispose(GObject* object) {
  g_return_if_fail(FL_IS_KEYBOARD_CALL_RECORD(object));

  FlKeyboardCallRecord* self = FL_KEYBOARD_CALL_RECORD(object);
  fl_key_event_dispose(self->event);
  G_OBJECT_CLASS(fl_keyboard_call_record_parent_class)->dispose(object);
}

// Class Initialization method for FlKeyboardCallRecord class.
static void fl_keyboard_call_record_class_init(
    FlKeyboardCallRecordClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_call_record_dispose;
}

static FlKeyboardCallRecord* fl_keyboard_call_record_new(
    FlKeyMockResponder* responder,
    FlKeyEvent* event,
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

/***** End FlKeyboardCallRecord *****/

static void dont_respond(FlKeyResponderAsyncCallback callback,
                         gpointer user_data) {}
static void respond_true(FlKeyResponderAsyncCallback callback,
                         gpointer user_data) {
  callback(true, user_data);
}
static void respond_false(FlKeyResponderAsyncCallback callback,
                          gpointer user_data) {
  callback(false, user_data);
}

static gboolean filter_keypress_returns_true(FlTextInputPlugin* self,
                                             FlKeyEvent* event) {
  return TRUE;
}

static gboolean filter_keypress_returns_false(FlTextInputPlugin* self,
                                              FlKeyEvent* event) {
  return FALSE;
}

/***** FlKeyMockResponder *****/

static void fl_key_mock_responder_iface_init(FlKeyResponderInterface* iface);

G_DEFINE_TYPE_WITH_CODE(FlKeyMockResponder,
                        fl_key_mock_responder,
                        G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(FL_TYPE_KEY_RESPONDER,
                                              fl_key_mock_responder_iface_init))

static void fl_key_mock_responder_handle_event(
    FlKeyResponder* responder,
    FlKeyEvent* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data);

static void fl_key_mock_responder_iface_init(FlKeyResponderInterface* iface) {
  iface->handle_event = fl_key_mock_responder_handle_event;
}

/***** End FlKeyMockResponder *****/

// Return a newly allocated #FlKeyEvent that is a clone to the given #event
// but with #origin and #dispose set to 0.
static FlKeyEvent* fl_key_event_clone_information_only(FlKeyEvent* event) {
  FlKeyEvent* new_event = fl_key_event_clone(event);
  new_event->origin = nullptr;
  new_event->dispose_origin = nullptr;
  return new_event;
}
static void fl_key_mock_responder_handle_event(
    FlKeyResponder* responder,
    FlKeyEvent* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data) {
  FlKeyMockResponder* self = FL_KEY_MOCK_RESPONDER(responder);
  g_ptr_array_add(self->call_records,
                  FL_KEYBOARD_CALL_RECORD(fl_keyboard_call_record_new(
                      self, fl_key_event_clone_information_only(event),
                      callback, user_data)));
  self->callback_handler(callback, user_data);
}

static void fl_key_mock_responder_class_init(FlKeyMockResponderClass* klass) {}

static void fl_key_mock_responder_init(FlKeyMockResponder* self) {}

static FlKeyMockResponder* fl_key_mock_responder_new(GPtrArray* call_records,
                                                     int delegate_id) {
  FlKeyMockResponder* self = FL_KEY_MOCK_RESPONDER(
      g_object_new(fl_key_mock_responder_get_type(), nullptr));

  self->call_records = call_records;
  self->callback_handler = dont_respond;
  self->delegate_id = delegate_id;

  return self;
}

static void g_ptr_array_clear(GPtrArray* array) {
  g_ptr_array_remove_range(array, 0, array->len);
}

static void fl_key_event_free_origin_by_mock(gpointer origin) {
  g_free(origin);
}
// Create a new #FlKeyEvent with the given information.
//
// The #origin will be another #FlKeyEvent with the exact information,
// so that it can be used to redispatch, and is freed upon disposal.
static FlKeyEvent* fl_key_event_new_by_mock(bool is_press,
                                            guint keyval,
                                            guint16 keycode,
                                            int state,
                                            gboolean is_modifier) {
  FlKeyEvent* event = g_new(FlKeyEvent, 1);
  event->is_press = is_press;
  event->time = 0;
  event->state = state;
  event->keyval = keyval;
  event->string = nullptr;
  event->keycode = keycode;
  FlKeyEvent* origin_event = fl_key_event_clone_information_only(event);
  event->origin = origin_event;
  event->dispose_origin = fl_key_event_free_origin_by_mock;
  return event;
}

class KeyboardTester {
 public:
  KeyboardTester(FlTextInputPlugin* text_input_plugin) {
    manager_ =
        fl_keyboard_manager_new(text_input_plugin, [this](gpointer event) {
          if (real_redispatcher_ != nullptr) {
            real_redispatcher_(event);
          }
        });
  }

  ~KeyboardTester() { g_clear_object(&manager_); }

  FlKeyboardManager* manager() { return manager_; }

  void recordRedispatchedEventsTo(std::vector<gpointer>& storage) {
    real_redispatcher_ = [&storage](gpointer event) {
      storage.push_back(event);
    };
  }

 private:
  FlKeyboardManager* manager_;
  FlKeyboardManagerRedispatcher real_redispatcher_;
};

// Make sure that the keyboard can be disposed without crashes when there are
// unresolved pending events.
TEST(FlKeyboardManagerTest, DisposeWithUnresolvedPends) {
  KeyboardTester tester(nullptr);

  GPtrArray* call_records = g_ptr_array_new_with_free_func(g_object_unref);
  FlKeyMockResponder* responder = fl_key_mock_responder_new(call_records, 1);
  fl_keyboard_manager_add_responder(tester.manager(),
                                    FL_KEY_RESPONDER(responder));

  responder->callback_handler = dont_respond;
  fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(true, GDK_KEY_a, kKeyCodeKeyA, 0x0, false));

  responder->callback_handler = respond_true;
  fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(false, GDK_KEY_a, kKeyCodeKeyA, 0x0, false));

  g_ptr_array_unref(call_records);

  // Passes if the cleanup does not crash.
}

TEST(FlKeyboardManagerTest, SingleDelegateWithAsyncResponds) {
  KeyboardTester tester(nullptr);
  std::vector<gpointer> redispatched;
  GPtrArray* call_records = g_ptr_array_new_with_free_func(g_object_unref);
  FlKeyboardCallRecord* record;

  gboolean manager_handled = false;
  fl_keyboard_manager_add_responder(
      tester.manager(),
      FL_KEY_RESPONDER(fl_key_mock_responder_new(call_records, 1)));

  /// Test 1: One event that is handled by the framework
  tester.recordRedispatchedEventsTo(redispatched);

  // Dispatch a key event
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(true, GDK_KEY_a, kKeyCodeKeyA, 0x0, false));
  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(redispatched.size(), 0u);
  EXPECT_EQ(call_records->len, 1u);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 0));
  EXPECT_EQ(record->responder->delegate_id, 1);
  EXPECT_EQ(record->event->keyval, 0x61u);
  EXPECT_EQ(record->event->keycode, 0x26u);

  record->callback(true, record->user_data);
  EXPECT_EQ(redispatched.size(), 0u);

  EXPECT_TRUE(fl_keyboard_manager_is_state_clear(tester.manager()));
  g_ptr_array_clear(call_records);

  /// Test 2: Two events that are unhandled by the framework
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(false, GDK_KEY_a, kKeyCodeKeyA, 0x0, false));
  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(redispatched.size(), 0u);
  EXPECT_EQ(call_records->len, 1u);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 0));
  EXPECT_EQ(record->responder->delegate_id, 1);
  EXPECT_EQ(record->event->keyval, 0x61u);
  EXPECT_EQ(record->event->keycode, 0x26u);

  // Dispatch another key event
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(true, GDK_KEY_b, kKeyCodeKeyB, 0x0, false));
  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(redispatched.size(), 0u);
  EXPECT_EQ(call_records->len, 2u);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 1));
  EXPECT_EQ(record->responder->delegate_id, 1);
  EXPECT_EQ(record->event->keyval, 0x62u);
  EXPECT_EQ(record->event->keycode, 0x38u);

  // Resolve the second event first to test out-of-order response
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 1));
  record->callback(false, record->user_data);
  EXPECT_EQ(redispatched.size(), 1u);
  EXPECT_EQ(FL_KEY_EVENT(redispatched.back())->keyval, 0x62u);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 0));
  record->callback(false, record->user_data);
  EXPECT_EQ(redispatched.size(), 2u);
  EXPECT_EQ(FL_KEY_EVENT(redispatched.back())->keyval, 0x61u);

  g_ptr_array_clear(call_records);

  // Resolve redispatches
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(), FL_KEY_EVENT(redispatched[0]));
  EXPECT_EQ(manager_handled, false);
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(), FL_KEY_EVENT(redispatched[1]));
  EXPECT_EQ(manager_handled, false);
  EXPECT_EQ(call_records->len, 0u);

  redispatched.clear();
  EXPECT_TRUE(fl_keyboard_manager_is_state_clear(tester.manager()));

  /// Test 3: Dispatch the same event again to ensure that prevention from
  /// redispatching only works once.
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(true, GDK_KEY_a, kKeyCodeKeyA, 0x0, false));
  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(redispatched.size(), 0u);
  EXPECT_EQ(call_records->len, 1u);

  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 0));
  record->callback(true, record->user_data);

  redispatched.clear();
  EXPECT_TRUE(fl_keyboard_manager_is_state_clear(tester.manager()));
  g_ptr_array_unref(call_records);
}

TEST(FlKeyboardManagerTest, SingleDelegateWithSyncResponds) {
  KeyboardTester tester(nullptr);
  std::vector<gpointer> redispatched;
  GPtrArray* call_records = g_ptr_array_new_with_free_func(g_object_unref);
  FlKeyboardCallRecord* record;

  gboolean manager_handled = false;
  FlKeyMockResponder* responder = fl_key_mock_responder_new(call_records, 1);
  fl_keyboard_manager_add_responder(tester.manager(),
                                    FL_KEY_RESPONDER(responder));

  /// Test 1: One event that is handled by the framework
  tester.recordRedispatchedEventsTo(redispatched);

  // Dispatch a key event
  responder->callback_handler = respond_true;
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(true, GDK_KEY_a, kKeyCodeKeyA, 0x0, false));
  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(call_records->len, 1u);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 0));
  EXPECT_EQ(record->responder->delegate_id, 1);
  EXPECT_EQ(record->event->keyval, 0x61u);
  EXPECT_EQ(record->event->keycode, 0x26u);
  EXPECT_EQ(redispatched.size(), 0u);

  EXPECT_TRUE(fl_keyboard_manager_is_state_clear(tester.manager()));
  g_ptr_array_clear(call_records);

  /// Test 2: An event unhandled by the framework
  responder->callback_handler = respond_false;
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(false, GDK_KEY_a, kKeyCodeKeyA, 0x0, false));
  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(call_records->len, 1u);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 0));
  EXPECT_EQ(record->responder->delegate_id, 1);
  EXPECT_EQ(record->event->keyval, 0x61u);
  EXPECT_EQ(record->event->keycode, 0x26u);
  EXPECT_EQ(redispatched.size(), 1u);

  g_ptr_array_clear(call_records);

  // Resolve redispatch
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(), FL_KEY_EVENT(redispatched[0]));
  EXPECT_EQ(manager_handled, false);
  EXPECT_EQ(call_records->len, 0u);

  EXPECT_TRUE(fl_keyboard_manager_is_state_clear(tester.manager()));
  redispatched.clear();
  g_ptr_array_unref(call_records);
}

TEST(FlKeyboardManagerTest, WithTwoAsyncDelegates) {
  KeyboardTester tester(nullptr);
  std::vector<gpointer> redispatched;
  GPtrArray* call_records = g_ptr_array_new_with_free_func(g_object_unref);
  FlKeyboardCallRecord* record;

  gboolean manager_handled = false;
  fl_keyboard_manager_add_responder(
      tester.manager(),
      FL_KEY_RESPONDER(fl_key_mock_responder_new(call_records, 1)));
  fl_keyboard_manager_add_responder(
      tester.manager(),
      FL_KEY_RESPONDER(fl_key_mock_responder_new(call_records, 2)));

  tester.recordRedispatchedEventsTo(redispatched);

  /// Test 1: One delegate responds true, the other false

  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(true, GDK_KEY_a, kKeyCodeKeyA, 0x0, false));

  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(redispatched.size(), 0u);
  EXPECT_EQ(call_records->len, 2u);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 0));
  EXPECT_EQ(record->responder->delegate_id, 1);
  EXPECT_EQ(record->event->keyval, 0x61u);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 1));
  EXPECT_EQ(record->responder->delegate_id, 2);
  EXPECT_EQ(record->event->keyval, 0x61u);

  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 0));
  record->callback(true, record->user_data);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 1));
  record->callback(false, record->user_data);
  EXPECT_EQ(redispatched.size(), 0u);

  EXPECT_TRUE(fl_keyboard_manager_is_state_clear(tester.manager()));
  g_ptr_array_clear(call_records);

  /// Test 2: All delegates respond false
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(false, GDK_KEY_a, kKeyCodeKeyA, 0x0, false));

  EXPECT_EQ(manager_handled, true);
  EXPECT_EQ(redispatched.size(), 0u);
  EXPECT_EQ(call_records->len, 2u);

  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 0));
  record->callback(false, record->user_data);
  EXPECT_EQ(redispatched.size(), 0u);
  record = FL_KEYBOARD_CALL_RECORD(g_ptr_array_index(call_records, 1));
  record->callback(false, record->user_data);
  EXPECT_EQ(redispatched.size(), 1u);

  g_ptr_array_clear(call_records);

  // Resolve redispatch
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(), FL_KEY_EVENT(redispatched[0]));
  EXPECT_EQ(manager_handled, false);
  EXPECT_EQ(call_records->len, 0u);

  EXPECT_TRUE(fl_keyboard_manager_is_state_clear(tester.manager()));
  g_ptr_array_clear(call_records);

  redispatched.clear();
  g_ptr_array_unref(call_records);
}

TEST(FlKeyboardManagerTest, TextInputPluginReturnsFalse) {
  // The text input plugin doesn't handle events.
  KeyboardTester tester(FL_TEXT_INPUT_PLUGIN(
      fl_mock_text_input_plugin_new(filter_keypress_returns_false)));
  std::vector<gpointer> redispatched;
  GPtrArray* call_records = g_ptr_array_new_with_free_func(g_object_unref);

  gboolean manager_handled = false;
  tester.recordRedispatchedEventsTo(redispatched);

  // The responder never handles events.
  FlKeyMockResponder* responder = fl_key_mock_responder_new(call_records, 1);
  fl_keyboard_manager_add_responder(tester.manager(),
                                    FL_KEY_RESPONDER(responder));
  responder->callback_handler = respond_false;

  // Dispatch a key event.
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(true, GDK_KEY_a, kKeyCodeKeyA, 0, false));
  EXPECT_EQ(manager_handled, true);
  // The event was redispatched because no one handles it.
  EXPECT_EQ(redispatched.size(), 1u);

  // Resolve redispatched event.
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(), FL_KEY_EVENT(redispatched[0]));
  EXPECT_EQ(manager_handled, false);

  redispatched.clear();
  EXPECT_TRUE(fl_keyboard_manager_is_state_clear(tester.manager()));
  g_ptr_array_unref(call_records);
}

TEST(FlKeyboardManagerTest, TextInputPluginReturnsTrue) {
  KeyboardTester tester(FL_TEXT_INPUT_PLUGIN(
      fl_mock_text_input_plugin_new(filter_keypress_returns_true)));
  std::vector<gpointer> redispatched;
  GPtrArray* call_records = g_ptr_array_new_with_free_func(g_object_unref);

  gboolean manager_handled = false;
  tester.recordRedispatchedEventsTo(redispatched);

  // The responder never handles events.
  FlKeyMockResponder* responder = fl_key_mock_responder_new(call_records, 1);
  fl_keyboard_manager_add_responder(tester.manager(),
                                    FL_KEY_RESPONDER(responder));
  responder->callback_handler = respond_false;

  // Dispatch a key event.
  manager_handled = fl_keyboard_manager_handle_event(
      tester.manager(),
      fl_key_event_new_by_mock(true, GDK_KEY_a, kKeyCodeKeyA, 0, false));
  EXPECT_EQ(manager_handled, true);
  // The event was not redispatched because text input plugin handles it.
  EXPECT_EQ(redispatched.size(), 0u);

  EXPECT_TRUE(fl_keyboard_manager_is_state_clear(tester.manager()));
  g_ptr_array_unref(call_records);
}

}  // namespace
