// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_embedder_responder.h"

#include "gtest/gtest.h"

#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/shell/platform/linux/fl_binary_messenger_private.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"

namespace {
constexpr gboolean kRelease = FALSE;
constexpr gboolean kPress = TRUE;

constexpr gboolean kIsModifier = TRUE;
constexpr gboolean kIsNotModifier = FALSE;

constexpr guint16 kKeyCodeKeyA = 0x26u;
constexpr guint16 kKeyCodeShiftRight = 0x3eu;
// constexpr guint16 kKeyCodeKeyB = 0x38u;

constexpr uint64_t kPhysicalKeyA = 0x00070004;
// constexpr uint64_t kPhysicalKeyQ = 0x00070014;
// constexpr uint64_t kPhysicalControlLeft = 0x000700e0;
// constexpr uint64_t kPhysicalControlRight = 0x000700e4;
// constexpr uint64_t kPhysicalShiftLeft = 0x000700e1;
constexpr uint64_t kPhysicalShiftRight = 0x000700e5;
// constexpr uint64_t kPhysicalKeyNumLock = 0x00070053;

constexpr uint64_t kLogicalKeyA = 0x00000061;
constexpr uint64_t kLogicalKeyQ = 0x00000071;
// constexpr uint64_t kLogicalControlLeft = 0x00300000105;
// constexpr uint64_t kLogicalControlRight = 0x00400000105;
// constexpr uint64_t kLogicalShiftLeft = 0x0030000010d;
constexpr uint64_t kLogicalShiftRight = 0x0040000010d;
// constexpr uint64_t kLogicalKeyNumLock = 0x0000000010a;
}  // namespace

static void g_ptr_array_clear(GPtrArray* array) {
  g_ptr_array_remove_range(array, 0, array->len);
}

G_DECLARE_FINAL_TYPE(FlKeyEmbedderCallRecord,
                     fl_key_embedder_call_record,
                     FL,
                     KEY_EMBEDDER_CALL_RECORD,
                     GObject);

struct _FlKeyEmbedderCallRecord {
  GObject parent_instance;

  FlutterKeyEvent* event;
  FlutterKeyEventCallback callback;
  gpointer user_data;
};

G_DEFINE_TYPE(FlKeyEmbedderCallRecord,
              fl_key_embedder_call_record,
              G_TYPE_OBJECT)

static void fl_key_embedder_call_record_init(FlKeyEmbedderCallRecord* self) {}

// Dispose method for FlKeyEmbedderCallRecord.
static void fl_key_embedder_call_record_dispose(GObject* object) {
  g_return_if_fail(FL_IS_KEY_EMBEDDER_CALL_RECORD(object));

  FlKeyEmbedderCallRecord* self = FL_KEY_EMBEDDER_CALL_RECORD(object);
  if (self->event != nullptr) {
    g_free(const_cast<char*>(self->event->character));
    g_free(self->event);
  }
  G_OBJECT_CLASS(fl_key_embedder_call_record_parent_class)->dispose(object);
}

// Class Initialization method for FlKeyEmbedderCallRecord class.
static void fl_key_embedder_call_record_class_init(
    FlKeyEmbedderCallRecordClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_key_embedder_call_record_dispose;
}

static FlKeyEmbedderCallRecord* fl_key_embedder_call_record_new(
    const FlutterKeyEvent* event,
    FlutterKeyEventCallback callback,
    gpointer user_data) {
  g_return_val_if_fail(event != nullptr, nullptr);

  FlKeyEmbedderCallRecord* self = FL_KEY_EMBEDDER_CALL_RECORD(
      g_object_new(fl_key_embedder_call_record_get_type(), nullptr));

  FlutterKeyEvent* clone_event = g_new(FlutterKeyEvent, 1);
  *clone_event = *event;
  if (event->character != nullptr) {
    size_t character_length = strlen(event->character);
    char* clone_character = g_new(char, character_length + 1);
    strcpy(clone_character, event->character);
    clone_event->character = clone_character;
  }
  self->event = clone_event;
  self->callback = callback;
  self->user_data = user_data;

  return self;
}

namespace {
// A global variable to store new event. It is a global variable so that it can
// be returned by key_event_new for easy use.
GdkEventKey _g_key_event;
}  // namespace

static GdkEventKey* key_event_new(guint32 time_in_milliseconds,
                                  gboolean is_down,
                                  guint keyval,
                                  guint16 hardware_keycode,
                                  guint state,
                                  gboolean is_modifier) {
  _g_key_event.type = is_down ? GDK_KEY_PRESS : GDK_KEY_RELEASE;
  _g_key_event.window = nullptr;
  _g_key_event.send_event = FALSE;
  _g_key_event.time = time_in_milliseconds;
  _g_key_event.state = state;
  _g_key_event.keyval = keyval;
  _g_key_event.length = 0;
  _g_key_event.string = nullptr;
  _g_key_event.hardware_keycode = hardware_keycode;
  _g_key_event.group = 0;
  _g_key_event.is_modifier = is_modifier ? 1 : 0;
  return &_g_key_event;
}

static gboolean g_expected_handled;
static gpointer g_expected_user_data;

static void responder_callback(bool handled, gpointer user_data) {
  EXPECT_EQ(handled, g_expected_handled);
}

static void invoke_record_callback_and_verify(FlKeyEmbedderCallRecord* record,
                                              bool expected_handled,
                                              void* expected_user_data) {
  g_expected_handled = expected_handled;
  g_expected_user_data = expected_user_data;
  record->callback(expected_handled, record->user_data);
}

namespace {
GPtrArray* g_call_records;
}

static FlEngine* make_mock_engine_with_records() {
  FlEngine* engine = make_mock_engine();
  FlutterEngineProcTable* embedder_api = fl_engine_get_embedder_api(engine);
  embedder_api->SendKeyEvent = [](auto engine, const FlutterKeyEvent* event,
                                  FlutterKeyEventCallback callback,
                                  void* user_data) {
    g_ptr_array_add(g_call_records, fl_key_embedder_call_record_new(
                                        event, callback, user_data));

    return kSuccess;
  };

  return engine;
}

// Basic key presses
TEST(FlKeyEmbedderResponderTest, SendKeyEvent) {
  EXPECT_EQ(g_call_records, nullptr);
  g_call_records = g_ptr_array_new_with_free_func(g_object_unref);
  g_autoptr(FlEngine) engine = make_mock_engine_with_records();
  g_autoptr(FlKeyResponder) responder =
      FL_KEY_RESPONDER(fl_key_embedder_responder_new(engine));
  int user_data = 123;  // Arbitrary user data

  FlKeyEmbedderCallRecord* record;

  // On a QWERTY keyboard, press key Q (physically key A), and release.
  // Key down
  fl_key_responder_handle_event(
      responder,
      key_event_new(12345, kPress, GDK_KEY_a, kKeyCodeKeyA, 0x10,
                    kIsNotModifier),
      responder_callback, &user_data);

  EXPECT_EQ(g_call_records->len, 1u);
  record = FL_KEY_EMBEDDER_CALL_RECORD(g_ptr_array_index(g_call_records, 0));
  EXPECT_EQ(record->event->struct_size, sizeof(FlutterKeyEvent));
  EXPECT_EQ(record->event->timestamp, 12345000);
  EXPECT_EQ(record->event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(record->event->physical, kPhysicalKeyA);
  EXPECT_EQ(record->event->logical, kLogicalKeyA);
  EXPECT_STREQ(record->event->character, "a");
  EXPECT_EQ(record->event->synthesized, false);

  invoke_record_callback_and_verify(record, TRUE, &user_data);
  g_ptr_array_clear(g_call_records);

  // Skip testing key repeats, which is not present on GDK.

  // Key up
  fl_key_responder_handle_event(
      responder,
      key_event_new(12346, kRelease, GDK_KEY_a, kKeyCodeKeyA, 0x10,
                    kIsNotModifier),
      responder_callback, &user_data);

  EXPECT_EQ(g_call_records->len, 1u);
  record = FL_KEY_EMBEDDER_CALL_RECORD(g_ptr_array_index(g_call_records, 0));
  EXPECT_EQ(record->event->struct_size, sizeof(FlutterKeyEvent));
  EXPECT_EQ(record->event->timestamp, 12346000);
  EXPECT_EQ(record->event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(record->event->physical, kPhysicalKeyA);
  EXPECT_EQ(record->event->logical, kLogicalKeyA);
  EXPECT_STREQ(record->event->character, nullptr);
  EXPECT_EQ(record->event->synthesized, false);

  invoke_record_callback_and_verify(record, FALSE, &user_data);
  g_ptr_array_clear(g_call_records);

  // On an AZERTY keyboard, press key Q (physically key A), and release.
  // Key down
  fl_key_responder_handle_event(
      responder,
      key_event_new(12347, kPress, GDK_KEY_q, kKeyCodeKeyA, 0x10,
                    kIsNotModifier),
      responder_callback, &user_data);

  EXPECT_EQ(g_call_records->len, 1u);
  record = FL_KEY_EMBEDDER_CALL_RECORD(g_ptr_array_index(g_call_records, 0));
  EXPECT_EQ(record->event->struct_size, sizeof(FlutterKeyEvent));
  EXPECT_EQ(record->event->timestamp, 12347000);
  EXPECT_EQ(record->event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(record->event->physical, kPhysicalKeyA);
  EXPECT_EQ(record->event->logical, kLogicalKeyQ);
  EXPECT_STREQ(record->event->character, "q");
  EXPECT_EQ(record->event->synthesized, false);

  invoke_record_callback_and_verify(record, TRUE, &user_data);
  g_ptr_array_clear(g_call_records);

  // Key up
  fl_key_responder_handle_event(
      responder,
      key_event_new(12348, kRelease, GDK_KEY_q, kKeyCodeKeyA, 0x10,
                    kIsNotModifier),
      responder_callback, &user_data);

  EXPECT_EQ(g_call_records->len, 1u);
  record = FL_KEY_EMBEDDER_CALL_RECORD(g_ptr_array_index(g_call_records, 0));
  EXPECT_EQ(record->event->struct_size, sizeof(FlutterKeyEvent));
  EXPECT_EQ(record->event->timestamp, 12348000);
  EXPECT_EQ(record->event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(record->event->physical, kPhysicalKeyA);
  EXPECT_EQ(record->event->logical, kLogicalKeyQ);
  EXPECT_STREQ(record->event->character, nullptr);
  EXPECT_EQ(record->event->synthesized, false);

  invoke_record_callback_and_verify(record, FALSE, &user_data);
  g_ptr_array_clear(g_call_records);

  g_clear_object(&g_call_records);
}

// Press Shift, key A, then release Shift, key A.
TEST(FlKeyEmbedderResponderTest, PressShiftDuringLetterKeyTap) {
  EXPECT_EQ(g_call_records, nullptr);
  g_call_records = g_ptr_array_new_with_free_func(g_object_unref);
  g_autoptr(FlEngine) engine = make_mock_engine_with_records();
  g_autoptr(FlKeyResponder) responder =
      FL_KEY_RESPONDER(fl_key_embedder_responder_new(engine));
  int user_data = 123;  // Arbitrary user data

  FlKeyEmbedderCallRecord* record;

  // Press shift right
  fl_key_responder_handle_event(
      responder,
      key_event_new(101, kPress, GDK_KEY_Shift_R, kKeyCodeShiftRight, 0x10,
                    kIsModifier),
      responder_callback, &user_data);

  EXPECT_EQ(g_call_records->len, 1u);
  record = FL_KEY_EMBEDDER_CALL_RECORD(g_ptr_array_index(g_call_records, 0));
  EXPECT_EQ(record->event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(record->event->physical, kPhysicalShiftRight);
  EXPECT_EQ(record->event->logical, kLogicalShiftRight);
  EXPECT_STREQ(record->event->character, nullptr);
  EXPECT_EQ(record->event->synthesized, false);

  invoke_record_callback_and_verify(record, TRUE, &user_data);
  g_ptr_array_clear(g_call_records);

  // Press key A
  fl_key_responder_handle_event(
      responder,
      key_event_new(102, kPress, GDK_KEY_A, kKeyCodeKeyA, 0x11, kIsNotModifier),
      responder_callback, &user_data);

  EXPECT_EQ(g_call_records->len, 1u);
  record = FL_KEY_EMBEDDER_CALL_RECORD(g_ptr_array_index(g_call_records, 0));
  EXPECT_EQ(record->event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(record->event->physical, kPhysicalKeyA);
  EXPECT_EQ(record->event->logical, kLogicalKeyA);
  EXPECT_STREQ(record->event->character, "A");
  EXPECT_EQ(record->event->synthesized, false);

  invoke_record_callback_and_verify(record, TRUE, &user_data);
  g_ptr_array_clear(g_call_records);

  // Release shift right
  fl_key_responder_handle_event(
      responder,
      key_event_new(103, kRelease, GDK_KEY_Shift_R, kKeyCodeShiftRight, 0x11,
                    kIsModifier),
      responder_callback, &user_data);

  EXPECT_EQ(g_call_records->len, 1u);
  record = FL_KEY_EMBEDDER_CALL_RECORD(g_ptr_array_index(g_call_records, 0));
  EXPECT_EQ(record->event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(record->event->physical, kPhysicalShiftRight);
  EXPECT_EQ(record->event->logical, kLogicalShiftRight);
  EXPECT_STREQ(record->event->character, nullptr);
  EXPECT_EQ(record->event->synthesized, false);

  invoke_record_callback_and_verify(record, TRUE, &user_data);
  g_ptr_array_clear(g_call_records);

  // Release key A
  fl_key_responder_handle_event(
      responder,
      key_event_new(104, kRelease, GDK_KEY_A, kKeyCodeKeyA, 0x10,
                    kIsNotModifier),
      responder_callback, &user_data);

  EXPECT_EQ(g_call_records->len, 1u);
  record = FL_KEY_EMBEDDER_CALL_RECORD(g_ptr_array_index(g_call_records, 0));
  EXPECT_EQ(record->event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(record->event->physical, kPhysicalKeyA);
  EXPECT_EQ(record->event->logical, kLogicalKeyA);
  EXPECT_STREQ(record->event->character, nullptr);
  EXPECT_EQ(record->event->synthesized, false);

  invoke_record_callback_and_verify(record, TRUE, &user_data);
  g_ptr_array_clear(g_call_records);

  g_clear_object(&g_call_records);
}
