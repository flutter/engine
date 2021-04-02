// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_manager.h"

#include <cstring>

#include "gtest/gtest.h"

G_BEGIN_DECLS

#define FL_KEY_MOCK_RESPONDER fl_key_mock_responder_get_type ()
G_DECLARE_FINAL_TYPE(FlKeyMockResponder,
                     fl_key_mock_responder,
                     FL,
                     KEY_MOCK_RESPONDER,
                     GObject);

G_END_DECLS

struct _FlKeyMockResponder {
  GObject parent_instance;

  FlKeyboardManager* manager;
  FlBasicMessageChannel* channel;
  GPtrArray* pending_events;
  uint64_t last_id;
};

static void fl_key_mock_responder_iface_init(
    FlKeyResponderInterface* iface);

G_DEFINE_TYPE_WITH_CODE(
  FlKeyMockResponder,
  fl_key_mock_responder,
  G_TYPE_OBJECT,
  G_IMPLEMENT_INTERFACE(FL_TYPE_KEY_RESPONDER,
                        fl_key_mock_responder_iface_init))

static bool fl_key_mock_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data);

static void fl_key_mock_responder_iface_init(
    FlKeyResponderInterface* iface) {
  iface->handle_event = fl_key_mock_responder_handle_event;
}

static bool fl_key_mock_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data) {

}

G_DECLARE_FINAL_TYPE(FlKeyboardCallRecord,
                     fl_keyboard_call_record,
                     FL,
                     KEYBOARD_CALL_RECORD,
                     GObject);

struct _FlKeyboardCallRecord {
  GdkEventKey* event;

  uint64_t hash;
};

G_DEFINE_TYPE(FlKeyboardCallRecord, fl_keyboard_call_record, G_TYPE_OBJECT)

// Dispose method for FlKeyboardCallRecord.
static void fl_keyboard_call_record_dispose(GObject* object) {
  // Redundant, but added so that we don't get a warning about unused function
  // for FL_IS_KEYBOARD_CALL_RECORD.
  g_return_if_fail(FL_IS_KEYBOARD_CALL_RECORD(object));

  FlKeyboardCallRecord* self = fl_keyboard_call_record(object);
  g_clear_pointer(&self->event, gdk_event_free);
  G_OBJECT_CLASS(fl_keyboard_call_record_parent_class)->dispose(object);
}

// Class Initialization method for FlKeyboardCallRecord class.
static void fl_keyboard_call_record_class_init(FlKeyboardCallRecordClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_call_record_dispose;
}

// Converts a binary blob to a string.
static gchar* message_to_text(GBytes* message) {
  size_t data_length;
  const gchar* data =
      static_cast<const gchar*>(g_bytes_get_data(message, &data_length));
  return g_strndup(data, data_length);
}

// Converts a string to a binary blob.
static GBytes* text_to_message(const gchar* text) {
  return g_bytes_new(text, strlen(text));
}

// Encodes a method call using JsonMethodCodec to a UTF-8 string.
static gchar* encode_method_call(const gchar* name, FlValue* args) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_method_call(
      FL_METHOD_CODEC(codec), name, args, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  return message_to_text(message);
}

// Encodes a success envelope response using JsonMethodCodec to a UTF-8 string.
static gchar* encode_success_envelope(FlValue* result) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_success_envelope(
      FL_METHOD_CODEC(codec), result, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  return message_to_text(message);
}

// Encodes a error envelope response using JsonMethodCodec to a UTF8 string.
static gchar* encode_error_envelope(const gchar* error_code,
                                    const gchar* error_message,
                                    FlValue* details) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_error_envelope(
      FL_METHOD_CODEC(codec), error_code, error_message, details, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  return message_to_text(message);
}

// Decodes a method call using JsonMethodCodec with a UTF8 string.
static void decode_method_call(const char* text, gchar** name, FlValue** args) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GBytes) data = text_to_message(text);
  g_autoptr(GError) error = nullptr;
  gboolean result = fl_method_codec_decode_method_call(
      FL_METHOD_CODEC(codec), data, name, args, &error);
  EXPECT_TRUE(result);
  EXPECT_EQ(error, nullptr);
}

// Decodes a method call using JsonMethodCodec. Expect the given error.
static void decode_error_method_call(const char* text,
                                     GQuark domain,
                                     gint code) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GBytes) data = text_to_message(text);
  g_autoptr(GError) error = nullptr;
  g_autofree gchar* name = nullptr;
  g_autoptr(FlValue) args = nullptr;
  gboolean result = fl_method_codec_decode_method_call(
      FL_METHOD_CODEC(codec), data, &name, &args, &error);
  EXPECT_FALSE(result);
  EXPECT_EQ(name, nullptr);
  EXPECT_EQ(args, nullptr);
  EXPECT_TRUE(g_error_matches(error, domain, code));
}

// Decodes a response using JsonMethodCodec. Expect the response is a result.
static void decode_response_with_success(const char* text, FlValue* result) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GBytes) message = text_to_message(text);
  g_autoptr(GError) error = nullptr;
  g_autoptr(FlMethodResponse) response =
      fl_method_codec_decode_response(FL_METHOD_CODEC(codec), message, &error);
  ASSERT_NE(response, nullptr);
  EXPECT_EQ(error, nullptr);
  ASSERT_TRUE(FL_IS_METHOD_SUCCESS_RESPONSE(response));
  EXPECT_TRUE(fl_value_equal(fl_method_success_response_get_result(
                                 FL_METHOD_SUCCESS_RESPONSE(response)),
                             result));
}

// Decodes a response using JsonMethodCodec. Expect the response contains the
// given error.
static void decode_response_with_error(const char* text,
                                       const gchar* code,
                                       const gchar* error_message,
                                       FlValue* details) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GBytes) message = text_to_message(text);
  g_autoptr(GError) error = nullptr;
  g_autoptr(FlMethodResponse) response =
      fl_method_codec_decode_response(FL_METHOD_CODEC(codec), message, &error);
  ASSERT_NE(response, nullptr);
  EXPECT_EQ(error, nullptr);
  ASSERT_TRUE(FL_IS_METHOD_ERROR_RESPONSE(response));
  EXPECT_STREQ(
      fl_method_error_response_get_code(FL_METHOD_ERROR_RESPONSE(response)),
      code);
  if (error_message == nullptr) {
    EXPECT_EQ(fl_method_error_response_get_message(
                  FL_METHOD_ERROR_RESPONSE(response)),
              nullptr);
  } else {
    EXPECT_STREQ(fl_method_error_response_get_message(
                     FL_METHOD_ERROR_RESPONSE(response)),
                 error_message);
  }
  if (details == nullptr) {
    EXPECT_EQ(fl_method_error_response_get_details(
                  FL_METHOD_ERROR_RESPONSE(response)),
              nullptr);
  } else {
    EXPECT_TRUE(fl_value_equal(fl_method_error_response_get_details(
                                   FL_METHOD_ERROR_RESPONSE(response)),
                               details));
  }
}

// Decode a response using JsonMethodCodec. Expect the given error.
static void decode_error_response(const char* text, GQuark domain, gint code) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GBytes) message = text_to_message(text);
  g_autoptr(GError) error = nullptr;
  g_autoptr(FlMethodResponse) response =
      fl_method_codec_decode_response(FL_METHOD_CODEC(codec), message, &error);
  EXPECT_EQ(response, nullptr);
  EXPECT_TRUE(g_error_matches(error, domain, code));
}

TEST(FlJsonMethodCodecTest, EncodeMethodCallNullptrArgs) {
  g_autofree gchar* text = encode_method_call("hello", nullptr);
  EXPECT_STREQ(text, "{\"method\":\"hello\",\"args\":null}");
}

TEST(FlKeyboardManagerTest, SingleDelegateWithAsyncResponds) {
  g_autolist(call_history)

  std::list<MockKeyHandlerDelegate::KeyboardHookCall> ;

  // Capture the scancode of the last redispatched event
  int redispatch_scancode = 0;
  bool delegate_handled = false;
  TestKeyboardKeyHandler handler([&redispatch_scancode](UINT cInputs,
                                                        LPINPUT pInputs,
                                                        int cbSize) -> UINT {
    EXPECT_TRUE(cbSize > 0);
    redispatch_scancode = pInputs->ki.wScan;
    return 1;
  });
  // Add one delegate
  auto delegate = std::make_unique<MockKeyHandlerDelegate>(1, &hook_history);
  handler.AddDelegate(std::move(delegate));

  /// Test 1: One event that is handled by the framework

  // Dispatch a key event
  delegate_handled = handler.KeyboardHook(nullptr, 64, kHandledScanCode,
                                          WM_KEYDOWN, L'a', false, true);
  EXPECT_EQ(delegate_handled, true);
  EXPECT_EQ(redispatch_scancode, 0);
  EXPECT_EQ(hook_history.size(), 1);
  EXPECT_EQ(hook_history.back().delegate_id, 1);
  EXPECT_EQ(hook_history.back().scancode, kHandledScanCode);
  EXPECT_EQ(hook_history.back().was_down, true);

  EXPECT_EQ(handler.HasRedispatched(), false);
  hook_history.back().callback(true);
  EXPECT_EQ(redispatch_scancode, 0);

  EXPECT_EQ(handler.HasRedispatched(), false);
  hook_history.clear();

  /// Test 2: Two events that are unhandled by the framework

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

  EXPECT_EQ(handler.HasRedispatched(), false);
  hook_history.clear();
  redispatch_scancode = 0;
}
