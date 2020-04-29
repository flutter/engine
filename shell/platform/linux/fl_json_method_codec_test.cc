// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_json_method_codec.h"
#include "flutter/shell/platform/linux/fl_method_codec_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_codec.h"
#include "gtest/gtest.h"

static gchar* message_to_text(GBytes* message) {
  size_t data_length;
  const gchar* data =
      static_cast<const gchar*>(g_bytes_get_data(message, &data_length));
  return g_strndup(data, data_length);
}

static GBytes* text_to_message(const gchar* text) {
  return g_bytes_new(text, strlen(text));
}

static gchar* encode_method_call(const gchar* name, FlValue* args) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GError) error = NULL;
  g_autoptr(GBytes) message = fl_method_codec_encode_method_call(
      FL_METHOD_CODEC(codec), name, args, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  return message_to_text(message);
}

static gchar* encode_success_envelope(FlValue* result) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GError) error = NULL;
  g_autoptr(GBytes) message = fl_method_codec_encode_success_envelope(
      FL_METHOD_CODEC(codec), result, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  return message_to_text(message);
}

static gchar* encode_error_envelope(const gchar* error_code,
                                    const gchar* error_message,
                                    FlValue* details) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GError) error = NULL;
  g_autoptr(GBytes) message = fl_method_codec_encode_error_envelope(
      FL_METHOD_CODEC(codec), error_code, error_message, details, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  return message_to_text(message);
}

static void decode_method_call(const char* text, gchar** name, FlValue** args) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GBytes) data = text_to_message(text);
  g_autoptr(GError) error = NULL;
  gboolean result = fl_method_codec_decode_method_call(
      FL_METHOD_CODEC(codec), data, name, args, &error);
  EXPECT_TRUE(result);
  EXPECT_EQ(error, nullptr);
}

static void decode_error_method_call(const char* text,
                                     GQuark domain,
                                     gint code) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GBytes) data = text_to_message(text);
  g_autoptr(GError) error = NULL;
  g_autofree gchar* name = NULL;
  g_autoptr(FlValue) args = NULL;
  gboolean result = fl_method_codec_decode_method_call(
      FL_METHOD_CODEC(codec), data, &name, &args, &error);
  EXPECT_FALSE(result);
  EXPECT_EQ(name, nullptr);
  EXPECT_EQ(args, nullptr);
  EXPECT_TRUE(g_error_matches(error, domain, code));
}

static void decode_response(const char* text,
                            gchar** error_code,
                            gchar** error_message,
                            FlValue** result_or_details) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GBytes) data = text_to_message(text);
  g_autoptr(GError) error = NULL;
  gboolean result =
      fl_method_codec_decode_response(FL_METHOD_CODEC(codec), data, error_code,
                                      error_message, result_or_details, &error);
  EXPECT_TRUE(result);
  EXPECT_EQ(error, nullptr);
}

static void decode_response_no_error(const char* text, FlValue** out_result) {
  g_autoptr(GError) error = NULL;
  g_autofree gchar* error_code = NULL;
  g_autofree gchar* error_message = NULL;
  g_autoptr(FlValue) result_or_details = NULL;
  decode_response(text, &error_code, &error_message, &result_or_details);
  EXPECT_EQ(error_code, nullptr);
  EXPECT_EQ(error_message, nullptr);
  EXPECT_NE(result_or_details, nullptr);
  EXPECT_EQ(error, nullptr);
  *out_result = fl_value_ref(result_or_details);
}

static void decode_response_with_error(const char* text,
                                       const gchar* code,
                                       gchar** message,
                                       FlValue** details) {
  g_autoptr(GError) error = NULL;
  g_autofree gchar* error_code = NULL;
  g_autofree gchar* error_message = NULL;
  g_autoptr(FlValue) result_or_details = NULL;
  decode_response(text, &error_code, &error_message, &result_or_details);
  EXPECT_NE(error_code, nullptr);
  EXPECT_STREQ(error_code, code);
  EXPECT_EQ(error, nullptr);

  *message = g_strdup(error_message);
  *details = fl_value_ref(result_or_details);
}

static void decode_error_response(const char* text, GQuark domain, gint code) {
  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  g_autoptr(GBytes) data = text_to_message(text);
  g_autoptr(GError) error = NULL;
  g_autofree gchar* error_code = NULL;
  g_autofree gchar* error_message = NULL;
  g_autoptr(FlValue) result_or_details = NULL;
  gboolean result = fl_method_codec_decode_response(
      FL_METHOD_CODEC(codec), data, &error_code, &error_message,
      &result_or_details, &error);
  EXPECT_FALSE(result);
  EXPECT_EQ(error_code, nullptr);
  EXPECT_EQ(error_message, nullptr);
  EXPECT_EQ(result_or_details, nullptr);
  EXPECT_TRUE(g_error_matches(error, domain, code));
}

TEST(FlJsonMethodCodecTest, EncodeMethodCallNULLArgs) {
  g_autofree gchar* text = encode_method_call("hello", NULL);
  EXPECT_STREQ(text, "{\"method\":\"hello\",\"args\":null}");
}

TEST(FlJsonMethodCodecTest, EncodeMethodCallNullArgs) {
  g_autoptr(FlValue) value = fl_value_null_new();
  g_autofree gchar* text = encode_method_call("hello", value);
  EXPECT_STREQ(text, "{\"method\":\"hello\",\"args\":null}");
}

TEST(FlJsonMethodCodecTest, EncodeMethodCallStringArgs) {
  g_autoptr(FlValue) args = fl_value_string_new("world");
  g_autofree gchar* text = encode_method_call("hello", args);
  EXPECT_STREQ(text, "{\"method\":\"hello\",\"args\":\"world\"}");
}

TEST(FlJsonMethodCodecTest, EncodeMethodCallListArgs) {
  g_autoptr(FlValue) args = fl_value_list_new();
  fl_value_list_add_take(args, fl_value_string_new("count"));
  fl_value_list_add_take(args, fl_value_int_new(42));
  g_autofree gchar* text = encode_method_call("hello", args);
  EXPECT_STREQ(text, "{\"method\":\"hello\",\"args\":[\"count\",42]}");
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallNoArgs) {
  g_autofree gchar* name = NULL;
  g_autoptr(FlValue) args = NULL;
  decode_method_call("{\"method\":\"hello\"}", &name, &args);
  EXPECT_STREQ(name, "hello");
  ASSERT_EQ(args, nullptr);
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallNullArgs) {
  g_autofree gchar* name = NULL;
  g_autoptr(FlValue) args = NULL;
  decode_method_call("{\"method\":\"hello\",\"args\":null}", &name, &args);
  EXPECT_STREQ(name, "hello");
  ASSERT_EQ(fl_value_get_type(args), FL_VALUE_TYPE_NULL);
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallStringArgs) {
  g_autofree gchar* name = NULL;
  g_autoptr(FlValue) args = NULL;
  decode_method_call("{\"method\":\"hello\",\"args\":\"world\"}", &name, &args);
  EXPECT_STREQ(name, "hello");
  ASSERT_EQ(fl_value_get_type(args), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(args), "world");
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallListArgs) {
  g_autofree gchar* name = NULL;
  g_autoptr(FlValue) args = NULL;
  decode_method_call("{\"method\":\"hello\",\"args\":[\"count\",42]}", &name,
                     &args);
  EXPECT_STREQ(name, "hello");
  ASSERT_EQ(fl_value_get_type(args), FL_VALUE_TYPE_LIST);
  EXPECT_EQ(fl_value_get_length(args), static_cast<size_t>(2));

  FlValue* arg0 = fl_value_list_get_value(args, 0);
  ASSERT_EQ(fl_value_get_type(arg0), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(arg0), "count");

  FlValue* arg1 = fl_value_list_get_value(args, 1);
  ASSERT_EQ(fl_value_get_type(arg1), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(arg1), 42);
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallNoData) {
  decode_error_method_call("", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallNoMethodOrArgs) {
  decode_error_method_call("{}", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallInvalidJson) {
  decode_error_method_call("X", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallWrongType) {
  decode_error_method_call("42", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallNoMethod) {
  decode_error_method_call("{\"args\":\"world\"}", FL_CODEC_ERROR,
                           FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallNoTerminator) {
  decode_error_method_call("{\"method\":\"hello\",\"args\":\"world\"",
                           FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMethodCodecTest, DecodeMethodCallExtraData) {
  decode_error_method_call("{\"method\":\"hello\"}XXX", FL_CODEC_ERROR,
                           FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonMethodCodecTest, EncodeSuccessEnvelopeNULL) {
  g_autofree gchar* text = encode_success_envelope(NULL);
  EXPECT_STREQ(text, "[null]");
}

TEST(FlJsonMethodCodecTest, EncodeSuccessEnvelopeNull) {
  g_autoptr(FlValue) result = fl_value_null_new();
  g_autofree gchar* text = encode_success_envelope(result);
  EXPECT_STREQ(text, "[null]");
}

TEST(FlJsonMethodCodecTest, EncodeSuccessEnvelopeString) {
  g_autoptr(FlValue) result = fl_value_string_new("hello");
  g_autofree gchar* text = encode_success_envelope(result);
  EXPECT_STREQ(text, "[\"hello\"]");
}

TEST(FlJsonMethodCodecTest, EncodeSuccessEnvelopeList) {
  g_autoptr(FlValue) result = fl_value_list_new();
  fl_value_list_add_take(result, fl_value_string_new("count"));
  fl_value_list_add_take(result, fl_value_int_new(42));
  g_autofree gchar* text = encode_success_envelope(result);
  EXPECT_STREQ(text, "[[\"count\",42]]");
}

TEST(FlJsonMethodCodecTest, EncodeErrorEnvelopeEmptyCode) {
  g_autofree gchar* text = encode_error_envelope("", NULL, NULL);
  EXPECT_STREQ(text, "[\"\",null,null]");
}

TEST(FlJsonMethodCodecTest, EncodeErrorEnvelopeNonMessageOrDetails) {
  g_autofree gchar* text = encode_error_envelope("error", NULL, NULL);
  EXPECT_STREQ(text, "[\"error\",null,null]");
}

TEST(FlJsonMethodCodecTest, EncodeErrorEnvelopeMessage) {
  g_autofree gchar* text = encode_error_envelope("error", "message", NULL);
  EXPECT_STREQ(text, "[\"error\",\"message\",null]");
}

TEST(FlJsonMethodCodecTest, EncodeErrorEnvelopeDetails) {
  g_autoptr(FlValue) details = fl_value_list_new();
  fl_value_list_add_take(details, fl_value_string_new("count"));
  fl_value_list_add_take(details, fl_value_int_new(42));
  g_autofree gchar* text = encode_error_envelope("error", NULL, details);
  EXPECT_STREQ(text, "[\"error\",null,[\"count\",42]]");
}

TEST(FlJsonMethodCodecTest, EncodeErrorEnvelopeMessageAndDetails) {
  g_autoptr(FlValue) details = fl_value_list_new();
  fl_value_list_add_take(details, fl_value_string_new("count"));
  fl_value_list_add_take(details, fl_value_int_new(42));
  g_autofree gchar* text = encode_error_envelope("error", "message", details);
  EXPECT_STREQ(text, "[\"error\",\"message\",[\"count\",42]]");
}

TEST(FlJsonMethodCodecTest, DecodeResponseSuccessNull) {
  g_autoptr(FlValue) result = NULL;
  decode_response_no_error("[null]", &result);
  ASSERT_EQ(fl_value_get_type(result), FL_VALUE_TYPE_NULL);
}

TEST(FlJsonMethodCodecTest, DecodeResponseSuccessString) {
  g_autoptr(FlValue) result = NULL;
  decode_response_no_error("[\"hello\"]", &result);
  ASSERT_EQ(fl_value_get_type(result), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(result), "hello");
}

TEST(FlJsonMethodCodecTest, DecodeResponseSuccessList) {
  g_autoptr(FlValue) result = NULL;
  decode_response_no_error("[[\"count\",42]]", &result);
  ASSERT_EQ(fl_value_get_type(result), FL_VALUE_TYPE_LIST);
  EXPECT_EQ(fl_value_get_length(result), static_cast<size_t>(2));

  FlValue* value0 = fl_value_list_get_value(result, 0);
  ASSERT_EQ(fl_value_get_type(value0), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value0), "count");

  FlValue* value1 = fl_value_list_get_value(result, 1);
  ASSERT_EQ(fl_value_get_type(value1), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value1), 42);
}

TEST(FlJsonMethodCodecTest, DecodeResponseErrorEmptyCode) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error("[\"\",null,null]", "", &message, &details);
  EXPECT_EQ(message, nullptr);
  ASSERT_EQ(fl_value_get_type(details), FL_VALUE_TYPE_NULL);
}

TEST(FlJsonMethodCodecTest, DecodeResponseErrorNoMessageOrDetails) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error("[\"error\",null,null]", "error", &message,
                             &details);
  EXPECT_EQ(message, nullptr);
  ASSERT_EQ(fl_value_get_type(details), FL_VALUE_TYPE_NULL);
}

TEST(FlJsonMethodCodecTest, DecodeResponseErrorMessage) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error("[\"error\",\"message\",null]", "error", &message,
                             &details);
  EXPECT_NE(message, nullptr);
  EXPECT_STREQ(message, "message");
  ASSERT_EQ(fl_value_get_type(details), FL_VALUE_TYPE_NULL);
}

TEST(FlJsonMethodCodecTest, DecodeResponseErrorDetails) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error("[\"error\",null,[\"count\",42]]", "error",
                             &message, &details);
  EXPECT_EQ(message, nullptr);
  ASSERT_EQ(fl_value_get_type(details), FL_VALUE_TYPE_LIST);
  EXPECT_EQ(fl_value_get_length(details), static_cast<size_t>(2));

  FlValue* value0 = fl_value_list_get_value(details, 0);
  ASSERT_EQ(fl_value_get_type(value0), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value0), "count");

  FlValue* value1 = fl_value_list_get_value(details, 1);
  ASSERT_EQ(fl_value_get_type(value1), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value1), 42);
}

TEST(FlJsonMethodCodecTest, DecodeResponseErrorMessageAndDetails) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error("[\"error\",\"message\",[\"count\",42]]", "error",
                             &message, &details);
  EXPECT_NE(message, nullptr);
  EXPECT_STREQ(message, "message");
  ASSERT_EQ(fl_value_get_type(details), FL_VALUE_TYPE_LIST);
  EXPECT_EQ(fl_value_get_length(details), static_cast<size_t>(2));

  FlValue* value0 = fl_value_list_get_value(details, 0);
  ASSERT_EQ(fl_value_get_type(value0), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value0), "count");

  FlValue* value1 = fl_value_list_get_value(details, 1);
  ASSERT_EQ(fl_value_get_type(value1), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value1), 42);
}

TEST(FlJsonMethodCodecTest, DecodeResponseNoData) {
  decode_error_response("", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMethodCodecTest, DecodeResponseNoTerminator) {
  decode_error_response("[42", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMethodCodecTest, DecodeResponseInvalidJson) {
  decode_error_response("X", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonMethodCodecTest, DecodeResponseMissingDetails) {
  decode_error_response("[\"error\",\"message\"]", FL_CODEC_ERROR,
                        FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonMethodCodecTest, DecodeResponseExtraDetails) {
  decode_error_response("[\"error\",\"message\",true,42]", FL_CODEC_ERROR,
                        FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonMethodCodecTest, DecodeResponseSuccessExtraData) {
  decode_error_response("[null]X", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonMethodCodecTest, DecodeResponseErrorExtraData) {
  decode_error_response("[\"error\",null,null]X", FL_CODEC_ERROR,
                        FL_CODEC_ERROR_FAILED);
}
