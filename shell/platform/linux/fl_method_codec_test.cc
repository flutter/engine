// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_codec.h"
#include "flutter/shell/platform/linux/fl_method_codec_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_codec.h"
#include "gtest/gtest.h"

G_DECLARE_FINAL_TYPE(FlTestMethodCodec,
                     fl_test_method_codec,
                     FL,
                     TEST_METHOD_CODEC,
                     FlMethodCodec)

struct _FlTestMethodCodec {
  FlMethodCodec parent_instance;
};

G_DEFINE_TYPE(FlTestMethodCodec,
              fl_test_method_codec,
              fl_method_codec_get_type())

static gchar* message_to_text(GBytes* message) {
  size_t data_length;
  const gchar* data =
      static_cast<const gchar*>(g_bytes_get_data(message, &data_length));
  return g_strndup(data, data_length);
}

static GBytes* text_to_message(const gchar* text) {
  return g_bytes_new(text, strlen(text));
}

static GBytes* fl_test_codec_encode_method_call(FlMethodCodec* codec,
                                                const gchar* name,
                                                FlValue* args,
                                                GError** error) {
  EXPECT_TRUE(FL_IS_TEST_METHOD_CODEC(codec));

  g_autofree gchar* text = nullptr;
  if (args == nullptr || fl_value_get_type(args) == FL_VALUE_TYPE_NULL)
    text = g_strdup_printf("%s()", name);
  else if (fl_value_get_type(args) == FL_VALUE_TYPE_INT)
    text = g_strdup_printf("%s(%" G_GINT64_FORMAT ")", name,
                           fl_value_get_int(args));
  else {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED, "ERROR");
    return nullptr;
  }

  return text_to_message(text);
}

static gboolean fl_test_codec_decode_method_call(FlMethodCodec* codec,
                                                 GBytes* message,
                                                 gchar** name,
                                                 FlValue** args,
                                                 GError** error) {
  EXPECT_TRUE(FL_IS_TEST_METHOD_CODEC(codec));

  g_autofree gchar* m = message_to_text(message);

  if (strcmp(m, "error") == 0) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED, "ERROR");
    return FALSE;
  } else {
    *name = g_strdup(m);
    *args = fl_value_null_new();
    return TRUE;
  }
}

static GBytes* fl_test_codec_encode_success_envelope(FlMethodCodec* codec,
                                                     FlValue* result,
                                                     GError** error) {
  EXPECT_TRUE(FL_IS_TEST_METHOD_CODEC(codec));

  g_autofree gchar* text = nullptr;
  if (result == nullptr || fl_value_get_type(result) == FL_VALUE_TYPE_NULL)
    text = g_strdup("(null)");
  else if (fl_value_get_type(result) == FL_VALUE_TYPE_INT)
    text = g_strdup_printf("%" G_GINT64_FORMAT, fl_value_get_int(result));
  else {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED, "ERROR");
    return nullptr;
  }

  return text_to_message(text);
}

static GBytes* fl_test_codec_encode_error_envelope(FlMethodCodec* codec,
                                                   const gchar* code,
                                                   const gchar* message,
                                                   FlValue* details,
                                                   GError** error) {
  EXPECT_TRUE(FL_IS_TEST_METHOD_CODEC(codec));

  if (details != nullptr && fl_value_get_type(details) != FL_VALUE_TYPE_INT) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED, "ERROR");
    return nullptr;
  }

  g_autofree gchar* text = nullptr;
  if (message == nullptr) {
    if (details == nullptr || fl_value_get_type(details) == FL_VALUE_TYPE_NULL)
      text = g_strdup_printf("Error_%s()", code);
    else
      text = g_strdup_printf("Error_%s(%" G_GINT64_FORMAT ")", code,
                             fl_value_get_int(details));
  } else {
    if (details == nullptr || fl_value_get_type(details) == FL_VALUE_TYPE_NULL)
      text = g_strdup_printf("Error_%s(%s)", code, message);
    else
      text = g_strdup_printf("Error_%s(%s,%" G_GINT64_FORMAT ")", code, message,
                             fl_value_get_int(details));
  }

  return text_to_message(text);
}

static gboolean fl_test_codec_decode_response(FlMethodCodec* codec,
                                              GBytes* message,
                                              gchar** error_code,
                                              gchar** error_message,
                                              FlValue** result,
                                              GError** error) {
  EXPECT_TRUE(FL_IS_TEST_METHOD_CODEC(codec));

  g_autofree gchar* m = message_to_text(message);
  if (strcmp(m, "codec-error") == 0) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED, "ERROR");
    return FALSE;
  } else if (strcmp(m, "error") == 0) {
    *error_code = g_strdup("code");
    *error_message = g_strdup("message");
    *result = fl_value_int_new(42);
    return TRUE;
  } else {
    *error_code = nullptr;
    *error_message = nullptr;
    *result = fl_value_string_new(m);
    return TRUE;
  }
}

static void fl_test_method_codec_class_init(FlTestMethodCodecClass* klass) {
  FL_METHOD_CODEC_CLASS(klass)->encode_method_call =
      fl_test_codec_encode_method_call;
  FL_METHOD_CODEC_CLASS(klass)->decode_method_call =
      fl_test_codec_decode_method_call;
  FL_METHOD_CODEC_CLASS(klass)->encode_success_envelope =
      fl_test_codec_encode_success_envelope;
  FL_METHOD_CODEC_CLASS(klass)->encode_error_envelope =
      fl_test_codec_encode_error_envelope;
  FL_METHOD_CODEC_CLASS(klass)->decode_response = fl_test_codec_decode_response;
}

static void fl_test_method_codec_init(FlTestMethodCodec* self) {}

static FlTestMethodCodec* fl_test_method_codec_new() {
  return FL_TEST_METHOD_CODEC(
      g_object_new(fl_test_method_codec_get_type(), nullptr));
}

TEST(FlMethodCodecTest, EncodeMethodCall) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_method_call(
      FL_METHOD_CODEC(codec), "foo", nullptr, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(message, nullptr);

  g_autofree gchar* message_text = message_to_text(message);
  EXPECT_STREQ(message_text, "foo()");
}

TEST(FlMethodCodecTest, EncodeMethodCallEmptyName) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_method_call(
      FL_METHOD_CODEC(codec), "", nullptr, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(message, nullptr);

  g_autofree gchar* message_text = message_to_text(message);
  EXPECT_STREQ(message_text, "()");
}

TEST(FlMethodCodecTest, EncodeMethodCallArgs) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(FlValue) args = fl_value_int_new(42);
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_method_call(
      FL_METHOD_CODEC(codec), "foo", args, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(message, nullptr);

  g_autofree gchar* message_text = message_to_text(message);
  EXPECT_STREQ(message_text, "foo(42)");
}

TEST(FlMethodCodecTest, EncodeMethodCallError) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(FlValue) args = fl_value_bool_new(FALSE);
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_method_call(
      FL_METHOD_CODEC(codec), "foo", args, &error);
  EXPECT_EQ(message, nullptr);
  EXPECT_TRUE(g_error_matches(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED));
}

TEST(FlMethodCodecTest, DecodeMethodCall) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(GBytes) message = text_to_message("foo");

  g_autofree gchar* name = nullptr;
  g_autoptr(FlValue) args = nullptr;
  g_autoptr(GError) error = nullptr;
  gboolean result = fl_method_codec_decode_method_call(
      FL_METHOD_CODEC(codec), message, &name, &args, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_TRUE(result);

  EXPECT_STREQ(name, "foo");
  ASSERT_EQ(fl_value_get_type(args), FL_VALUE_TYPE_NULL);
}

TEST(FlMethodCodecTest, EncodeSuccessEnvelope) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(FlValue) result = fl_value_int_new(42);
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_success_envelope(
      FL_METHOD_CODEC(codec), result, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(message, nullptr);

  g_autofree gchar* message_text = message_to_text(message);
  EXPECT_STREQ(message_text, "42");
}

TEST(FlMethodCodecTest, EncodeSuccessEnvelopeEmpty) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_success_envelope(
      FL_METHOD_CODEC(codec), nullptr, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(message, nullptr);

  g_autofree gchar* message_text = message_to_text(message);
  EXPECT_STREQ(message_text, "(null)");
}

TEST(FlMethodCodecTest, EncodeSuccessEnvelopeError) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(FlValue) result = fl_value_string_new("X");
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_success_envelope(
      FL_METHOD_CODEC(codec), result, &error);
  EXPECT_EQ(message, nullptr);
  EXPECT_TRUE(g_error_matches(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED));
}

TEST(FlMethodCodecTest, EncodeErrorEnvelopeNoMessageOrDetails) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_error_envelope(
      FL_METHOD_CODEC(codec), "code", nullptr, nullptr, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(message, nullptr);

  g_autofree gchar* message_text = message_to_text(message);
  EXPECT_STREQ(message_text, "Error_code()");
}

TEST(FlMethodCodecTest, EncodeErrorEnvelopeMessage) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_error_envelope(
      FL_METHOD_CODEC(codec), "code", "message", nullptr, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(message, nullptr);

  g_autofree gchar* message_text = message_to_text(message);
  EXPECT_STREQ(message_text, "Error_code(message)");
}

TEST(FlMethodCodecTest, EncodeErrorEnvelopeDetails) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(FlValue) details = fl_value_int_new(42);
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_error_envelope(
      FL_METHOD_CODEC(codec), "code", nullptr, details, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(message, nullptr);

  g_autofree gchar* message_text = message_to_text(message);
  EXPECT_STREQ(message_text, "Error_code(42)");
}

TEST(FlMethodCodecTest, EncodeErrorEnvelopeMessageAndDetails) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(FlValue) details = fl_value_int_new(42);
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_error_envelope(
      FL_METHOD_CODEC(codec), "code", "message", details, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(message, nullptr);

  g_autofree gchar* message_text = message_to_text(message);
  EXPECT_STREQ(message_text, "Error_code(message,42)");
}

TEST(FlMethodCodecTest, DecodeResponseSuccess) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(GBytes) message = text_to_message("echo");

  g_autofree gchar* error_code = nullptr;
  g_autofree gchar* error_message = nullptr;
  g_autoptr(FlValue) result_or_details = nullptr;
  g_autoptr(GError) error = nullptr;
  gboolean result = fl_method_codec_decode_response(
      FL_METHOD_CODEC(codec), message, &error_code, &error_message,
      &result_or_details, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_TRUE(result);
  EXPECT_EQ(error_code, nullptr);
  EXPECT_EQ(error_message, nullptr);

  ASSERT_EQ(fl_value_get_type(result_or_details), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(result_or_details), "echo");
}

TEST(FlMethodCodecTest, DecodeResponseCodecError) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(GBytes) message = text_to_message("codec-error");

  g_autofree gchar* error_code = nullptr;
  g_autofree gchar* error_message = nullptr;
  g_autoptr(FlValue) result_or_details = nullptr;
  g_autoptr(GError) error = nullptr;
  gboolean result = fl_method_codec_decode_response(
      FL_METHOD_CODEC(codec), message, &error_code, &error_message,
      &result_or_details, &error);
  EXPECT_EQ(error_code, nullptr);
  EXPECT_EQ(error_message, nullptr);
  EXPECT_EQ(result_or_details, nullptr);
  EXPECT_TRUE(g_error_matches(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED));
  EXPECT_FALSE(result);
}

TEST(FlMethodCodecTest, DecodeResponseError) {
  g_autoptr(FlTestMethodCodec) codec = fl_test_method_codec_new();

  g_autoptr(GBytes) message = text_to_message("error");

  g_autofree gchar* error_code = nullptr;
  g_autofree gchar* error_message = nullptr;
  g_autoptr(FlValue) details = nullptr;
  g_autoptr(GError) error = nullptr;
  gboolean result = fl_method_codec_decode_response(
      FL_METHOD_CODEC(codec), message, &error_code, &error_message, &details,
      &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_TRUE(result);
  EXPECT_STREQ(error_code, "code");
  EXPECT_STREQ(error_message, "message");

  ASSERT_EQ(fl_value_get_type(details), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(details), 42);
}
