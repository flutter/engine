// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_method_codec.h"
#include "flutter/shell/platform/linux/fl_method_codec_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_codec.h"
#include "gtest/gtest.h"

// FIXME(robert-ancell) Have two cases where endianess matters

static gchar* message_to_hex_string(GBytes* message) {
  GString* hex_string = g_string_new("");
  gsize data_length;
  const uint8_t* data =
      static_cast<const uint8_t*>(g_bytes_get_data(message, &data_length));
  for (guint i = 0; i < data_length; i++)
    g_string_append_printf(hex_string, "%02x", data[i]);
  return g_string_free(hex_string, FALSE);
}

static gchar* encode_method_call(const gchar* name, FlValue* args) {
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(GError) error = NULL;
  g_autoptr(GBytes) message = fl_method_codec_encode_method_call(
      FL_METHOD_CODEC(codec), name, args, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  return message_to_hex_string(message);
}

static gchar* encode_success_envelope(FlValue* result) {
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(GError) error = NULL;
  g_autoptr(GBytes) message = fl_method_codec_encode_success_envelope(
      FL_METHOD_CODEC(codec), result, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  return message_to_hex_string(message);
}

static gchar* encode_error_envelope(const gchar* error_code,
                                    const gchar* error_message,
                                    FlValue* details) {
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(GError) error = NULL;
  g_autoptr(GBytes) message = fl_method_codec_encode_error_envelope(
      FL_METHOD_CODEC(codec), error_code, error_message, details, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  return message_to_hex_string(message);
}

static uint8_t hex_digit_to_int(char value) {
  if (value >= '0' && value <= '9')
    return value - '0';
  else if (value >= 'a' && value <= 'f')
    return value - 'a' + 10;
  else if (value >= 'F' && value <= 'F')
    return value - 'A' + 10;
  else
    return 0;
}

static uint8_t parse_hex8(const gchar* hex_string) {
  if (hex_string[0] == '\0')
    return 0x00;
  return hex_digit_to_int(hex_string[0]) << 4 | hex_digit_to_int(hex_string[1]);
}

static GBytes* hex_string_to_bytes(const gchar* hex_string) {
  GByteArray* buffer = g_byte_array_new();
  for (int i = 0; hex_string[i] != '\0' && hex_string[i + 1] != '\0'; i += 2) {
    uint8_t value = parse_hex8(hex_string + i);
    g_byte_array_append(buffer, &value, 1);
  }
  return g_byte_array_free_to_bytes(buffer);
}

static void decode_method_call(const char* hex_string,
                               gchar** name,
                               FlValue** args) {
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(GBytes) data = hex_string_to_bytes(hex_string);
  g_autoptr(GError) error = NULL;
  gboolean result = fl_method_codec_decode_method_call(
      FL_METHOD_CODEC(codec), data, name, args, &error);
  EXPECT_TRUE(result);
  EXPECT_EQ(error, nullptr);
}

static void decode_error_method_call(const char* hex_string,
                                     GQuark domain,
                                     gint code) {
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(GBytes) data = hex_string_to_bytes(hex_string);
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

static void decode_response(const char* hex_string,
                            gchar** error_code,
                            gchar** error_message,
                            FlValue** result_or_details) {
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(GBytes) data = hex_string_to_bytes(hex_string);
  g_autoptr(GError) error = NULL;
  gboolean result =
      fl_method_codec_decode_response(FL_METHOD_CODEC(codec), data, error_code,
                                      error_message, result_or_details, &error);
  EXPECT_TRUE(result);
  EXPECT_EQ(error, nullptr);
}

static void decode_response_no_error(const char* hex_string,
                                     FlValue** out_result) {
  g_autoptr(GError) error = NULL;
  g_autofree gchar* error_code = NULL;
  g_autofree gchar* error_message = NULL;
  g_autoptr(FlValue) result_or_details = NULL;
  decode_response(hex_string, &error_code, &error_message, &result_or_details);
  EXPECT_EQ(error_code, nullptr);
  EXPECT_EQ(error_message, nullptr);
  EXPECT_NE(result_or_details, nullptr);
  EXPECT_EQ(error, nullptr);
  *out_result = fl_value_ref(result_or_details);
}

static void decode_response_with_error(const char* hex_string,
                                       const gchar* code,
                                       gchar** message,
                                       FlValue** details) {
  g_autoptr(GError) error = NULL;
  g_autofree gchar* error_code = NULL;
  g_autofree gchar* error_message = NULL;
  g_autoptr(FlValue) result_or_details = NULL;
  decode_response(hex_string, &error_code, &error_message, &result_or_details);
  EXPECT_NE(error_code, nullptr);
  EXPECT_STREQ(error_code, code);
  EXPECT_EQ(error, nullptr);

  *message = g_strdup(error_message);
  *details = fl_value_ref(result_or_details);
}

static void decode_error_response(const char* hex_string,
                                  GQuark domain,
                                  gint code) {
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(GBytes) data = hex_string_to_bytes(hex_string);
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

TEST(FlStandardMethodCodecTest, EncodeMethodCallNULLArgs) {
  g_autofree gchar* hex_string = encode_method_call("hello", NULL);
  EXPECT_STREQ(hex_string, "070568656c6c6f00");
}

TEST(FlStandardMethodCodecTest, EncodeMethodCallNullArgs) {
  g_autoptr(FlValue) value = fl_value_null_new();
  g_autofree gchar* hex_string = encode_method_call("hello", value);
  EXPECT_STREQ(hex_string, "070568656c6c6f00");
}

TEST(FlStandardMethodCodecTest, EncodeMethodCallStringArgs) {
  g_autoptr(FlValue) args = fl_value_string_new("world");
  g_autofree gchar* hex_string = encode_method_call("hello", args);
  EXPECT_STREQ(hex_string, "070568656c6c6f0705776f726c64");
}

TEST(FlStandardMethodCodecTest, EncodeMethodCallListArgs) {
  g_autoptr(FlValue) args = fl_value_list_new();
  fl_value_list_add_take(args, fl_value_string_new("count"));
  fl_value_list_add_take(args, fl_value_int_new(42));
  g_autofree gchar* hex_string = encode_method_call("hello", args);
  EXPECT_STREQ(hex_string, "070568656c6c6f0c020705636f756e74032a000000");
}

TEST(FlStandardMethodCodecTest, DecodeMethodCallNullArgs) {
  g_autofree gchar* name = NULL;
  g_autoptr(FlValue) args = NULL;
  decode_method_call("070568656c6c6f00", &name, &args);
  EXPECT_STREQ(name, "hello");
  ASSERT_EQ(fl_value_get_type(args), FL_VALUE_TYPE_NULL);
}

TEST(FlStandardMethodCodecTest, DecodeMethodCallStringArgs) {
  g_autofree gchar* name = NULL;
  g_autoptr(FlValue) args = NULL;
  decode_method_call("070568656c6c6f0705776f726c64", &name, &args);
  EXPECT_STREQ(name, "hello");
  ASSERT_EQ(fl_value_get_type(args), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(args), "world");
}

TEST(FlStandardMethodCodecTest, DecodeMethodCallListArgs) {
  g_autofree gchar* name = NULL;
  g_autoptr(FlValue) args = NULL;
  decode_method_call("070568656c6c6f0c020705636f756e74032a000000", &name,
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

TEST(FlStandardMethodCodecTest, DecodeMethodCallNoData) {
  decode_error_method_call("", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardMethodCodecTest, DecodeMethodCallNullMethodName) {
  decode_error_method_call("000000", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlStandardMethodCodecTest, DecodeMethodCallMissingArgs) {
  decode_error_method_call("070568656c6c6f", FL_CODEC_ERROR,
                           FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardMethodCodecTest, EncodeSuccessEnvelopeNULL) {
  g_autofree gchar* hex_string = encode_success_envelope(NULL);
  EXPECT_STREQ(hex_string, "0000");
}

TEST(FlStandardMethodCodecTest, EncodeSuccessEnvelopeNull) {
  g_autoptr(FlValue) result = fl_value_null_new();
  g_autofree gchar* hex_string = encode_success_envelope(result);
  EXPECT_STREQ(hex_string, "0000");
}

TEST(FlStandardMethodCodecTest, EncodeSuccessEnvelopeString) {
  g_autoptr(FlValue) result = fl_value_string_new("hello");
  g_autofree gchar* hex_string = encode_success_envelope(result);
  EXPECT_STREQ(hex_string, "00070568656c6c6f");
}

TEST(FlStandardMethodCodecTest, EncodeSuccessEnvelopeList) {
  g_autoptr(FlValue) result = fl_value_list_new();
  fl_value_list_add_take(result, fl_value_string_new("count"));
  fl_value_list_add_take(result, fl_value_int_new(42));
  g_autofree gchar* hex_string = encode_success_envelope(result);
  EXPECT_STREQ(hex_string, "000c020705636f756e74032a000000");
}

TEST(FlStandardMethodCodecTest, EncodeErrorEnvelopeEmptyCode) {
  g_autofree gchar* hex_string = encode_error_envelope("", NULL, NULL);
  EXPECT_STREQ(hex_string, "0107000000");
}

TEST(FlStandardMethodCodecTest, EncodeErrorEnvelopeNonMessageOrDetails) {
  g_autofree gchar* hex_string = encode_error_envelope("error", NULL, NULL);
  EXPECT_STREQ(hex_string, "0107056572726f720000");
}

TEST(FlStandardMethodCodecTest, EncodeErrorEnvelopeMessage) {
  g_autofree gchar* hex_string =
      encode_error_envelope("error", "message", NULL);
  EXPECT_STREQ(hex_string, "0107056572726f7207076d65737361676500");
}

TEST(FlStandardMethodCodecTest, EncodeErrorEnvelopeDetails) {
  g_autoptr(FlValue) details = fl_value_list_new();
  fl_value_list_add_take(details, fl_value_string_new("count"));
  fl_value_list_add_take(details, fl_value_int_new(42));
  g_autofree gchar* hex_string = encode_error_envelope("error", NULL, details);
  EXPECT_STREQ(hex_string, "0107056572726f72000c020705636f756e74032a000000");
}

TEST(FlStandardMethodCodecTest, EncodeErrorEnvelopeMessageAndDetails) {
  g_autoptr(FlValue) details = fl_value_list_new();
  fl_value_list_add_take(details, fl_value_string_new("count"));
  fl_value_list_add_take(details, fl_value_int_new(42));
  g_autofree gchar* hex_string =
      encode_error_envelope("error", "message", details);
  EXPECT_STREQ(
      hex_string,
      "0107056572726f7207076d6573736167650c020705636f756e74032a000000");
}

TEST(FlStandardMethodCodecTest, DecodeResponseSuccessNull) {
  g_autoptr(FlValue) result = NULL;
  decode_response_no_error("0000", &result);
  ASSERT_EQ(fl_value_get_type(result), FL_VALUE_TYPE_NULL);
}

TEST(FlStandardMethodCodecTest, DecodeResponseSuccessString) {
  g_autoptr(FlValue) result = NULL;
  decode_response_no_error("00070568656c6c6f", &result);
  ASSERT_EQ(fl_value_get_type(result), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(result), "hello");
}

TEST(FlStandardMethodCodecTest, DecodeResponseSuccessList) {
  g_autoptr(FlValue) result = NULL;
  decode_response_no_error("000c020705636f756e74032a000000", &result);
  ASSERT_EQ(fl_value_get_type(result), FL_VALUE_TYPE_LIST);
  EXPECT_EQ(fl_value_get_length(result), static_cast<size_t>(2));

  FlValue* value0 = fl_value_list_get_value(result, 0);
  ASSERT_EQ(fl_value_get_type(value0), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value0), "count");

  FlValue* value1 = fl_value_list_get_value(result, 1);
  ASSERT_EQ(fl_value_get_type(value1), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value1), 42);
}

TEST(FlStandardMethodCodecTest, DecodeResponseErrorEmptyCode) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error("0107000000", "", &message, &details);
  EXPECT_EQ(message, nullptr);
  ASSERT_EQ(fl_value_get_type(details), FL_VALUE_TYPE_NULL);
}

TEST(FlStandardMethodCodecTest, DecodeResponseErrorNoMessageOrDetails) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error("0107056572726f720000", "error", &message,
                             &details);
  EXPECT_EQ(message, nullptr);
  ASSERT_EQ(fl_value_get_type(details), FL_VALUE_TYPE_NULL);
}

TEST(FlStandardMethodCodecTest, DecodeResponseErrorMessage) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error("0107056572726f7207076d65737361676500", "error",
                             &message, &details);
  EXPECT_NE(message, nullptr);
  EXPECT_STREQ(message, "message");
  ASSERT_EQ(fl_value_get_type(details), FL_VALUE_TYPE_NULL);
}

TEST(FlStandardMethodCodecTest, DecodeResponseErrorDetails) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error("0107056572726f72000c020705636f756e74032a000000",
                             "error", &message, &details);
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

TEST(FlStandardMethodCodecTest, DecodeResponseErrorMessageAndDetails) {
  g_autofree gchar* message = NULL;
  g_autoptr(FlValue) details = NULL;
  decode_response_with_error(
      "0107056572726f7207076d6573736167650c020705636f756e74032a000000", "error",
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

TEST(FlStandardMethodCodecTest, DecodeResponseNoData) {
  decode_error_response("", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardMethodCodecTest, DecodeResponseSuccessNoData) {
  decode_error_response("00", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardMethodCodecTest, DecodeResponseSuccessExtraData) {
  decode_error_response("000000", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlStandardMethodCodecTest, DecodeResponseErrorNoData) {
  decode_error_response("01", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardMethodCodecTest, DecodeResponseErrorMissingMessageAndDetails) {
  decode_error_response("0107056572726f72", FL_CODEC_ERROR,
                        FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardMethodCodecTest, DecodeResponseErrorMissingDetails) {
  decode_error_response("0107056572726f7200", FL_CODEC_ERROR,
                        FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardMethodCodecTest, DecodeResponseErrorExtraData) {
  decode_error_response("0107056572726f72000000", FL_CODEC_ERROR,
                        FL_CODEC_ERROR_FAILED);
}

TEST(FlStandardMethodCodecTest, DecodeResponseUnknownEnvelope) {
  decode_error_response("02", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}
