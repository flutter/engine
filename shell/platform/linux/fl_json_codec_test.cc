// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_json_codec.h"
#include "gtest/gtest.h"

#include <math.h>

// FIXME: Test decoding integers that can't fit into 64 bits

static gchar* encode_value(FlValue* value) {
  g_autoptr(FlJsonCodec) codec = fl_json_codec_new();
  g_autoptr(GError) error = NULL;
  g_autofree gchar* result = fl_json_codec_encode(codec, value, &error);
  EXPECT_EQ(error, nullptr);
  return static_cast<gchar*>(g_steal_pointer(&result));
}

static FlValue* decode_value(const char* text) {
  g_autoptr(FlJsonCodec) codec = fl_json_codec_new();
  g_autoptr(GError) error = NULL;
  g_autoptr(FlValue) value = fl_json_codec_decode(codec, text, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(value, nullptr);
  return fl_value_ref(value);
}

static void decode_error_value(const char* text, GQuark domain, gint code) {
  g_autoptr(FlJsonCodec) codec = fl_json_codec_new();
  g_autoptr(GError) error = NULL;
  g_autoptr(FlValue) value = fl_json_codec_decode(codec, text, &error);
  EXPECT_TRUE(g_error_matches(error, domain, code));
  EXPECT_EQ(value, nullptr);
}

TEST(FlJsonCodecTest, EncodeNULL) {
  g_autofree gchar* text = encode_value(NULL);
  EXPECT_STREQ(text, "null");
}

TEST(FlJsonCodecTest, EncodeNull) {
  g_autoptr(FlValue) value = fl_value_null_new();
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "null");
}

TEST(FlJsonCodecTest, DecodeNull) {
  g_autoptr(FlValue) value = decode_value("null");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_NULL);
}

static gchar* encode_bool(gboolean value) {
  g_autoptr(FlValue) v = fl_value_bool_new(value);
  return encode_value(v);
}

TEST(FlJsonCodecTest, EncodeBoolFalse) {
  g_autofree gchar* text = encode_bool(FALSE);
  EXPECT_STREQ(text, "false");
}

TEST(FlJsonCodecTest, EncodeBoolTrue) {
  g_autofree gchar* text = encode_bool(TRUE);
  EXPECT_STREQ(text, "true");
}

TEST(FlJsonCodecTest, DecodeBoolFalse) {
  g_autoptr(FlValue) value = decode_value("false");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_BOOL);
  EXPECT_FALSE(fl_value_get_bool(value));
}

TEST(FlJsonCodecTest, DecodeBoolTrue) {
  g_autoptr(FlValue) value = decode_value("true");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_BOOL);
  EXPECT_TRUE(fl_value_get_bool(value));
}

static gchar* encode_int(int64_t value) {
  g_autoptr(FlValue) v = fl_value_int_new(value);
  return encode_value(v);
}

TEST(FlJsonCodecTest, EncodeIntZero) {
  g_autofree gchar* text = encode_int(0);
  EXPECT_STREQ(text, "0");
}

TEST(FlJsonCodecTest, EncodeIntOne) {
  g_autofree gchar* text = encode_int(1);
  EXPECT_STREQ(text, "1");
}

TEST(FlJsonCodecTest, EncodeInt12345) {
  g_autofree gchar* text = encode_int(12345);
  EXPECT_STREQ(text, "12345");
}

TEST(FlJsonCodecTest, EncodeIntMin) {
  g_autofree gchar* text = encode_int(G_MININT64);
  EXPECT_STREQ(text, "-9223372036854775808");
}

TEST(FlJsonCodecTest, EncodeIntMax) {
  g_autofree gchar* text = encode_int(G_MAXINT64);
  EXPECT_STREQ(text, "9223372036854775807");
}

TEST(FlJsonCodecTest, DecodeIntZero) {
  g_autoptr(FlValue) value = decode_value("0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 0);
}

TEST(FlJsonCodecTest, DecodeIntOne) {
  g_autoptr(FlValue) value = decode_value("1");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 1);
}

TEST(FlJsonCodecTest, DecodeInt12345) {
  g_autoptr(FlValue) value = decode_value("12345");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 12345);
}

TEST(FlJsonCodecTest, DecodeIntMin) {
  g_autoptr(FlValue) value = decode_value("-9223372036854775808");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), G_MININT64);
}

TEST(FlJsonCodecTest, DecodeIntMax) {
  g_autoptr(FlValue) value = decode_value("9223372036854775807");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), G_MAXINT64);
}

TEST(FlJsonCodecTest, DecodeIntLeadingZero1) {
  decode_error_value("00", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_UNUSED_DATA);
}

TEST(FlJsonCodecTest, DecodeIntLeadingZero2) {
  decode_error_value("01", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_UNUSED_DATA);
}

TEST(FlJsonCodecTest, DecodeIntDoubleNegative) {
  decode_error_value("--1", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_NUMBER);
}

TEST(FlJsonCodecTest, DecodeIntPositiveSign) {
  decode_error_value("+1", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonCodecTest, DecodeIntHexChar) {
  decode_error_value("0a", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_UNUSED_DATA);
}

static gchar* encode_float(double value) {
  g_autoptr(FlValue) v = fl_value_float_new(value);
  return encode_value(v);
}

TEST(FlJsonCodecTest, EncodeFloatZero) {
  g_autofree gchar* text = encode_float(0);
  EXPECT_STREQ(text, "0.0");
}

TEST(FlJsonCodecTest, EncodeFloatOne) {
  g_autofree gchar* text = encode_float(1);
  EXPECT_STREQ(text, "1.0");
}

TEST(FlJsonCodecTest, EncodeFloatMinusOne) {
  g_autofree gchar* text = encode_float(-1);
  EXPECT_STREQ(text, "-1.0");
}

TEST(FlJsonCodecTest, EncodeFloatHalf) {
  g_autofree gchar* text = encode_float(0.5);
  EXPECT_STREQ(text, "0.5");
}

TEST(FlJsonCodecTest, EncodeFloatPi) {
  g_autofree gchar* text = encode_float(M_PI);
  EXPECT_STREQ(text, "3.1415926535897931");
}

TEST(FlJsonCodecTest, EncodeFloatMinusZero) {
  g_autofree gchar* text = encode_float(-0.0);
  EXPECT_STREQ(text, "-0.0");
}

TEST(FlJsonCodecTest, EncodeFloatNaN) {
  g_autoptr(FlValue) value = fl_value_float_new(NAN);
  g_autoptr(FlJsonCodec) codec = fl_json_codec_new();
  g_autoptr(GError) error = NULL;
  g_autofree gchar* result = fl_json_codec_encode(codec, value, &error);
  EXPECT_TRUE(g_error_matches(error, FL_JSON_CODEC_ERROR,
                              FL_JSON_CODEC_ERROR_INVALID_NUMBER));
  EXPECT_EQ(result, nullptr);
}

TEST(FlJsonCodecTest, EncodeFloatInfinity) {
  g_autoptr(FlValue) value = fl_value_float_new(INFINITY);
  g_autoptr(FlJsonCodec) codec = fl_json_codec_new();
  g_autoptr(GError) error = NULL;
  g_autofree gchar* result = fl_json_codec_encode(codec, value, &error);
  EXPECT_TRUE(g_error_matches(error, FL_JSON_CODEC_ERROR,
                              FL_JSON_CODEC_ERROR_INVALID_NUMBER));
  EXPECT_EQ(result, nullptr);
}

TEST(FlJsonCodecTest, DecodeFloatZero) {
  g_autoptr(FlValue) value = decode_value("0.0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), 0.0);
}

TEST(FlJsonCodecTest, DecodeFloatOne) {
  g_autoptr(FlValue) value = decode_value("1.0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), 1.0);
}

TEST(FlJsonCodecTest, DecodeFloatMinusOne) {
  g_autoptr(FlValue) value = decode_value("-1.0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), -1.0);
}

TEST(FlJsonCodecTest, DecodeFloatHalf) {
  g_autoptr(FlValue) value = decode_value("0.5");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), 0.5);
}

TEST(FlJsonCodecTest, DecodeFloatPi) {
  g_autoptr(FlValue) value = decode_value("3.1415926535897931");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), M_PI);
}

TEST(FlJsonCodecTest, DecodeFloatMinusZero) {
  g_autoptr(FlValue) value = decode_value("-0.0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), -0.0);
}

TEST(FlJsonCodecTest, DecodeFloatMissingFraction) {
  decode_error_value("0.", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_NUMBER);
}

TEST(FlJsonCodecTest, DecodeFloatInvalidFraction) {
  decode_error_value("0.a", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_NUMBER);
}

static gchar* encode_string(const gchar* value) {
  g_autoptr(FlValue) v = fl_value_string_new(value);
  return encode_value(v);
}

TEST(FlJsonCodecTest, EncodeStringEmpty) {
  g_autofree gchar* text = encode_string("");
  EXPECT_STREQ(text, "\"\"");
}

TEST(FlJsonCodecTest, EncodeStringHello) {
  g_autofree gchar* text = encode_string("hello");
  EXPECT_STREQ(text, "\"hello\"");
}

TEST(FlJsonCodecTest, EncodeStringEmptySized) {
  g_autoptr(FlValue) value = fl_value_string_new_sized(NULL, 0);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "\"\"");
}

TEST(FlJsonCodecTest, EncodeStringHelloSized) {
  g_autoptr(FlValue) value = fl_value_string_new_sized("Hello World", 5);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "\"Hello\"");
}

TEST(FlJsonCodecTest, EncodeStringEscapeQuote) {
  g_autofree gchar* text = encode_string("\"");
  EXPECT_STREQ(text, "\"\\\"\"");
}

TEST(FlJsonCodecTest, EncodeStringEscapeBackslash) {
  g_autofree gchar* text = encode_string("\\");
  EXPECT_STREQ(text, "\"\\\\\"");
}

TEST(FlJsonCodecTest, EncodeStringEscapeBackspace) {
  g_autofree gchar* text = encode_string("\b");
  EXPECT_STREQ(text, "\"\\b\"");
}

TEST(FlJsonCodecTest, EncodeStringEscapeFormFeed) {
  g_autofree gchar* text = encode_string("\f");
  EXPECT_STREQ(text, "\"\\f\"");
}

TEST(FlJsonCodecTest, EncodeStringEscapeNewline) {
  g_autofree gchar* text = encode_string("\n");
  EXPECT_STREQ(text, "\"\\n\"");
}

TEST(FlJsonCodecTest, EncodeStringEscapeCarriageReturn) {
  g_autofree gchar* text = encode_string("\r");
  EXPECT_STREQ(text, "\"\\r\"");
}

TEST(FlJsonCodecTest, EncodeStringEscapeTab) {
  g_autofree gchar* text = encode_string("\t");
  EXPECT_STREQ(text, "\"\\t\"");
}

TEST(FlJsonCodecTest, EncodeStringEscapeUnicode) {
  g_autofree gchar* text = encode_string("\u0001");
  EXPECT_STREQ(text, "\"\\u0001\"");
}

TEST(FlJsonCodecTest, DecodeStringEmpty) {
  g_autoptr(FlValue) value = decode_value("\"\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "");
}

TEST(FlJsonCodecTest, DecodeStringHello) {
  g_autoptr(FlValue) value = decode_value("\"hello\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "hello");
}

TEST(FlJsonCodecTest, DecodeStringEscapeQuote) {
  g_autoptr(FlValue) value = decode_value("\"\\\"\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\"");
}

TEST(FlJsonCodecTest, DecodeStringEscapeBackslash) {
  g_autoptr(FlValue) value = decode_value("\"\\\\\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\\");
}

TEST(FlJsonCodecTest, DecodeStringEscapeSlash) {
  g_autoptr(FlValue) value = decode_value("\"\\/\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "/");
}

TEST(FlJsonCodecTest, DecodeStringEscapeBackspace) {
  g_autoptr(FlValue) value = decode_value("\"\\b\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\b");
}

TEST(FlJsonCodecTest, DecodeStringEscapeFormFeed) {
  g_autoptr(FlValue) value = decode_value("\"\\f\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\f");
}

TEST(FlJsonCodecTest, DecodeStringEscapeNewline) {
  g_autoptr(FlValue) value = decode_value("\"\\n\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\n");
}

TEST(FlJsonCodecTest, DecodeStringEscapeCarriageReturn) {
  g_autoptr(FlValue) value = decode_value("\"\\r\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\r");
}

TEST(FlJsonCodecTest, DecodeStringEscapeTab) {
  g_autoptr(FlValue) value = decode_value("\"\\t\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\t");
}

TEST(FlJsonCodecTest, DecodeStringEscapeUnicode) {
  g_autoptr(FlValue) value = decode_value("\"\\u0001\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\u0001");
}

TEST(FlJsonCodecTest, DecodeStringBinary) {
  decode_error_value("\"Hello\x01World\"", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_STRING_CHARACTER);
}

TEST(FlJsonCodecTest, DecodeStringNewline) {
  decode_error_value("\"Hello\nWorld\"", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_STRING_CHARACTER);
}

TEST(FlJsonCodecTest, DecodeStringCarriageReturn) {
  decode_error_value("\"Hello\rWorld\"", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_STRING_CHARACTER);
}

TEST(FlJsonCodecTest, DecodeStringTab) {
  decode_error_value("\"Hello\tWorld\"", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_STRING_CHARACTER);
}

TEST(FlJsonCodecTest, DecodeStringUnterminatedEmpty) {
  decode_error_value("\"", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonCodecTest, DecodeStringExtraQuote) {
  decode_error_value("\"\"\"", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_UNUSED_DATA);
}

TEST(FlJsonCodecTest, DecodeStringEscapedClosingQuote) {
  decode_error_value("\"\\\"", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonCodecTest, DecodeStringUnknownEscape) {
  decode_error_value("\"\\z\"", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_STRING_ESCAPE_SEQUENCE);
}

TEST(FlJsonCodecTest, DecodeStringInvalidUnicode) {
  decode_error_value("\"\\uxxxx\"", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_STRING_UNICODE_ESCAPE);
}

TEST(FlJsonCodecTest, DecodeStringUnicodeNoData) {
  decode_error_value("\"\\u\"", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_STRING_UNICODE_ESCAPE);
}

TEST(FlJsonCodecTest, DecodeStringUnicodeShortData) {
  decode_error_value("\"\\uxx\"", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_INVALID_STRING_UNICODE_ESCAPE);
}

TEST(FlJsonCodecTest, EncodeUint8ListEmpty) {
  g_autoptr(FlValue) value = fl_value_uint8_list_new(NULL, 0);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonCodecTest, EncodeUint8List) {
  uint8_t data[] = {0, 1, 2, 3, 4};
  g_autoptr(FlValue) value = fl_value_uint8_list_new(data, 5);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[0,1,2,3,4]");
}

TEST(FlJsonCodecTest, EncodeInt32ListEmpty) {
  g_autoptr(FlValue) value = fl_value_int32_list_new(NULL, 0);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonCodecTest, EncodeInt32List) {
  int32_t data[] = {0, -1, 2, -3, 4};
  g_autoptr(FlValue) value = fl_value_int32_list_new(data, 5);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[0,-1,2,-3,4]");
}

TEST(FlJsonCodecTest, EncodeInt64ListEmpty) {
  g_autoptr(FlValue) value = fl_value_int64_list_new(NULL, 0);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonCodecTest, EncodeInt64List) {
  int64_t data[] = {0, -1, 2, -3, 4};
  g_autoptr(FlValue) value = fl_value_int64_list_new(data, 5);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[0,-1,2,-3,4]");
}

TEST(FlJsonCodecTest, EncodeFloatListEmpty) {
  g_autoptr(FlValue) value = fl_value_float_list_new(NULL, 0);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonCodecTest, EncodeFloatList) {
  double data[] = {0, -0.5, 0.25, -0.125, 0.0625};
  g_autoptr(FlValue) value = fl_value_float_list_new(data, 5);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[0.0,-0.5,0.25,-0.125,0.0625]");
}

TEST(FlJsonCodecTest, EncodeListEmpty) {
  g_autoptr(FlValue) value = fl_value_list_new();
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonCodecTest, EncodeListTypes) {
  g_autoptr(FlValue) value = fl_value_list_new();
  fl_value_list_add_take(value, fl_value_null_new());
  fl_value_list_add_take(value, fl_value_bool_new(TRUE));
  fl_value_list_add_take(value, fl_value_int_new(42));
  fl_value_list_add_take(value, fl_value_float_new(-1.5));
  fl_value_list_add_take(value, fl_value_string_new("hello"));
  fl_value_list_add_take(value, fl_value_list_new());
  fl_value_list_add_take(value, fl_value_map_new());
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[null,true,42,-1.5,\"hello\",[],{}]");
}

TEST(FlJsonCodecTest, EncodeListNested) {
  g_autoptr(FlValue) even_numbers = fl_value_list_new();
  g_autoptr(FlValue) odd_numbers = fl_value_list_new();
  for (int i = 0; i < 10; i++) {
    if (i % 2 == 0)
      fl_value_list_add_take(even_numbers, fl_value_int_new(i));
    else
      fl_value_list_add_take(odd_numbers, fl_value_int_new(i));
  }
  g_autoptr(FlValue) value = fl_value_list_new();
  fl_value_list_add(value, even_numbers);
  fl_value_list_add(value, odd_numbers);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "[[0,2,4,6,8],[1,3,5,7,9]]");
}

TEST(FlJsonCodecTest, DecodeListEmpty) {
  g_autoptr(FlValue) value = decode_value("[]");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_LIST);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlJsonCodecTest, DecodeListNoComma) {
  decode_error_value("[0,1,2,3 4]", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_MISSING_COMMA);
}

TEST(FlJsonCodecTest, DecodeListUnterminatedEmpty) {
  decode_error_value("[", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonCodecTest, DecodeListStartUnterminate) {
  decode_error_value("]", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonCodecTest, DecodeListUnterminated) {
  decode_error_value("[0,1,2,3,4", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonCodecTest, DecodeListDoubleTerminated) {
  decode_error_value("[0,1,2,3,4]]", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_UNUSED_DATA);
}

TEST(FlJsonCodecTest, EncodeMapEmpty) {
  g_autoptr(FlValue) value = fl_value_map_new();
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text, "{}");
}

TEST(FlJsonCodecTest, EncodeMapKeyTypes) {
  g_autoptr(FlValue) value = fl_value_map_new();
  fl_value_map_set_take(value, fl_value_null_new(),
                        fl_value_string_new("null"));
  fl_value_map_set_take(value, fl_value_bool_new(TRUE),
                        fl_value_string_new("bool"));
  fl_value_map_set_take(value, fl_value_int_new(42),
                        fl_value_string_new("int"));
  fl_value_map_set_take(value, fl_value_float_new(-1.5),
                        fl_value_string_new("float"));
  fl_value_map_set_take(value, fl_value_string_new("hello"),
                        fl_value_string_new("string"));
  fl_value_map_set_take(value, fl_value_list_new(),
                        fl_value_string_new("list"));
  fl_value_map_set_take(value, fl_value_map_new(), fl_value_string_new("map"));
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text,
               "{null:\"null\",true:\"bool\",42:\"int\",-1.5:\"float\","
               "\"hello\":\"string\",[]:\"list\",{}:\"map\"}");
}

TEST(FlJsonCodecTest, EncodeMapValueTypes) {
  g_autoptr(FlValue) value = fl_value_map_new();
  fl_value_map_set_take(value, fl_value_string_new("null"),
                        fl_value_null_new());
  fl_value_map_set_take(value, fl_value_string_new("bool"),
                        fl_value_bool_new(TRUE));
  fl_value_map_set_take(value, fl_value_string_new("int"),
                        fl_value_int_new(42));
  fl_value_map_set_take(value, fl_value_string_new("float"),
                        fl_value_float_new(-1.5));
  fl_value_map_set_take(value, fl_value_string_new("string"),
                        fl_value_string_new("hello"));
  fl_value_map_set_take(value, fl_value_string_new("list"),
                        fl_value_list_new());
  fl_value_map_set_take(value, fl_value_string_new("map"), fl_value_map_new());
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text,
               "{\"null\":null,\"bool\":true,\"int\":42,\"float\":-"
               "1.5,\"string\":\"hello\",\"list\":[],\"map\":{}}");
}

TEST(FlJsonCodecTest, EncodeMapNested) {
  g_autoptr(FlValue) str_to_int = fl_value_map_new();
  g_autoptr(FlValue) int_to_str = fl_value_map_new();
  const char* numbers[] = {"zero", "one", "two", "three", NULL};
  for (int i = 0; numbers[i] != NULL; i++) {
    fl_value_map_set_take(str_to_int, fl_value_string_new(numbers[i]),
                          fl_value_int_new(i));
    fl_value_map_set_take(int_to_str, fl_value_int_new(i),
                          fl_value_string_new(numbers[i]));
  }
  g_autoptr(FlValue) value = fl_value_map_new();
  fl_value_map_set_string(value, "str-to-int", str_to_int);
  fl_value_map_set_string(value, "int-to-str", int_to_str);
  g_autofree gchar* text = encode_value(value);
  EXPECT_STREQ(text,
               "{\"str-to-int\":{\"zero\":0,\"one\":1,\"two\":2,\"three\":3},"
               "\"int-to-str\":{0:\"zero\",1:\"one\",2:\"two\",3:\"three\"}}");
}

TEST(FlJsonCodecTest, DecodeMapEmpty) {
  g_autoptr(FlValue) value = decode_value("{}");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_MAP);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlJsonCodecTest, DecodeMapUnterminatedEmpty) {
  decode_error_value("{", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonCodecTest, DecodeMapStartUnterminate) {
  decode_error_value("}", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonCodecTest, DecodeMapNoComma) {
  decode_error_value("{\"zero\":0 \"one\":1}", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_MISSING_COMMA);
}

TEST(FlJsonCodecTest, DecodeMapNoColon) {
  decode_error_value("{\"zero\" 0,\"one\":1}", FL_CODEC_ERROR,
                     FL_CODEC_ERROR_FAILED);
}

TEST(FlJsonCodecTest, DecodeMapUnterminated) {
  decode_error_value("{\"zero\":0,\"one\":1", FL_CODEC_ERROR,
                     FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonCodecTest, DecodeMapDoubleTerminated) {
  decode_error_value("{\"zero\":0,\"one\":1}}", FL_JSON_CODEC_ERROR,
                     FL_JSON_CODEC_ERROR_UNUSED_DATA);
}

TEST(FlJsonCodecTest, DecodeUnknownWord) {
  decode_error_value("foo", FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED);
}
