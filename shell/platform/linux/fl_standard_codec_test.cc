// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_codec.h"
#include "gtest/gtest.h"

// FIXME(robert-ancell) Have two cases where endianess matters
// FIXME(robert-ancell) Add tests for strings/lists/maps with more than 253
// elements

static gchar* encode_value(FlValue* value) {
  g_autoptr(FlStandardCodec) codec = fl_standard_codec_new();
  g_autoptr(GByteArray) buffer = g_byte_array_new();
  g_autoptr(GError) error = NULL;
  gboolean result =
      fl_codec_write_value(FL_CODEC(codec), buffer, value, &error);
  EXPECT_TRUE(result);
  EXPECT_EQ(error, nullptr);

  GString* hex_string = g_string_new("");
  for (guint i = 0; i < buffer->len; i++)
    g_string_append_printf(hex_string, "%02x", buffer->data[i]);
  return g_string_free(hex_string, FALSE);
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

static FlValue* decode_value(const char* hex_string) {
  g_autoptr(FlStandardCodec) codec = fl_standard_codec_new();
  g_autoptr(GBytes) data = hex_string_to_bytes(hex_string);
  size_t offset = 0;
  g_autoptr(GError) error = NULL;
  g_autoptr(FlValue) value =
      fl_codec_read_value(FL_CODEC(codec), data, &offset, &error);
  EXPECT_EQ(offset, g_bytes_get_size(data));
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(value, nullptr);
  return fl_value_ref(value);
}

static void decode_error_value(const char* hex_string,
                               GQuark domain,
                               gint code) {
  g_autoptr(FlStandardCodec) codec = fl_standard_codec_new();
  g_autoptr(GBytes) data = hex_string_to_bytes(hex_string);
  size_t offset = 0;
  g_autoptr(GError) error = NULL;
  g_autoptr(FlValue) value =
      fl_codec_read_value(FL_CODEC(codec), data, &offset, &error);
  EXPECT_TRUE(value == nullptr);
  EXPECT_TRUE(g_error_matches(error, domain, code));
  EXPECT_EQ(offset, static_cast<size_t>(0));
}

TEST(FlStandardCodecTest, EncodeNULL) {
  g_autofree gchar* hex_string = encode_value(NULL);
  EXPECT_STREQ(hex_string, "00");
}

TEST(FlStandardCodecTest, EncodeNull) {
  g_autoptr(FlValue) value = fl_value_null_new();
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "00");
}

static gchar* encode_bool(gboolean value) {
  g_autoptr(FlValue) v = fl_value_bool_new(value);
  return encode_value(v);
}

TEST(FlStandardCodecTest, EncodeBoolFalse) {
  g_autofree gchar* hex_string = encode_bool(FALSE);
  EXPECT_STREQ(hex_string, "02");
}

TEST(FlStandardCodecTest, EncodeBoolTrue) {
  g_autofree gchar* hex_string = encode_bool(TRUE);
  EXPECT_STREQ(hex_string, "01");
}

static gchar* encode_int(int64_t value) {
  g_autoptr(FlValue) v = fl_value_int_new(value);
  return encode_value(v);
}

TEST(FlStandardCodecTest, EncodeIntZero) {
  g_autofree gchar* hex_string = encode_int(0);
  EXPECT_STREQ(hex_string, "0300000000");
}

TEST(FlStandardCodecTest, EncodeIntOne) {
  g_autofree gchar* hex_string = encode_int(1);
  EXPECT_STREQ(hex_string, "0301000000");
}

TEST(FlStandardCodecTest, EncodeInt32) {
  g_autofree gchar* hex_string = encode_int(0x01234567);
  EXPECT_STREQ(hex_string, "0367452301");
}

TEST(FlStandardCodecTest, EncodeInt32Min) {
  g_autofree gchar* hex_string = encode_int(G_MININT32);
  EXPECT_STREQ(hex_string, "0300000080");
}

TEST(FlStandardCodecTest, EncodeInt32Max) {
  g_autofree gchar* hex_string = encode_int(G_MAXINT32);
  EXPECT_STREQ(hex_string, "03ffffff7f");
}

TEST(FlStandardCodecTest, EncodeInt64) {
  g_autofree gchar* hex_string = encode_int(0x0123456789abcdef);
  EXPECT_STREQ(hex_string, "04efcdab8967452301");
}

TEST(FlStandardCodecTest, EncodeInt64Min) {
  g_autofree gchar* hex_string = encode_int(G_MININT64);
  EXPECT_STREQ(hex_string, "040000000000000080");
}

TEST(FlStandardCodecTest, EncodeInt64Max) {
  g_autofree gchar* hex_string = encode_int(G_MAXINT64);
  EXPECT_STREQ(hex_string, "04ffffffffffffff7f");
}

TEST(FlStandardCodecTest, DecodeIntZero) {
  g_autoptr(FlValue) value = decode_value("0300000000");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 0);
}

TEST(FlStandardCodecTest, DecodeIntOne) {
  g_autoptr(FlValue) value = decode_value("0301000000");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 1);
}

TEST(FlStandardCodecTest, DecodeInt32) {
  g_autoptr(FlValue) value = decode_value("0367452301");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 0x01234567);
}

TEST(FlStandardCodecTest, DecodeInt32Min) {
  g_autoptr(FlValue) value = decode_value("0300000080");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), G_MININT32);
}

TEST(FlStandardCodecTest, DecodeInt32Max) {
  g_autoptr(FlValue) value = decode_value("03ffffff7f");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), G_MAXINT32);
}

TEST(FlStandardCodecTest, DecodeInt64) {
  g_autoptr(FlValue) value = decode_value("04efcdab8967452301");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 0x0123456789abcdef);
}

TEST(FlStandardCodecTest, DecodeInt64Min) {
  g_autoptr(FlValue) value = decode_value("040000000000000080");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), G_MININT64);
}

TEST(FlStandardCodecTest, DecodeInt64Max) {
  g_autoptr(FlValue) value = decode_value("04ffffffffffffff7f");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), G_MAXINT64);
}

TEST(FlStandardCodecTest, DecodeInt32NoData) {
  decode_error_value("03", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeIntShortData1) {
  decode_error_value("0367", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeIntShortData2) {
  decode_error_value("03674523", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeInt64NoData) {
  decode_error_value("04", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeInt64ShortData1) {
  decode_error_value("04ef", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeInt64ShortData2) {
  decode_error_value("04efcdab89674523", FL_CODEC_ERROR,
                     FL_CODEC_ERROR_OUT_OF_DATA);
}

static gchar* encode_float(double value) {
  g_autoptr(FlValue) v = fl_value_float_new(value);
  return encode_value(v);
}

TEST(FlStandardCodecTest, EncodeFloatZero) {
  g_autofree gchar* hex_string = encode_float(0);
  EXPECT_STREQ(hex_string, "060000000000000000");
}

TEST(FlStandardCodecTest, EncodeFloatOne) {
  g_autofree gchar* hex_string = encode_float(1);
  EXPECT_STREQ(hex_string, "06000000000000f03f");
}

TEST(FlStandardCodecTest, EncodeFloatMinusOne) {
  g_autofree gchar* hex_string = encode_float(-1);
  EXPECT_STREQ(hex_string, "06000000000000f0bf");
}

TEST(FlStandardCodecTest, EncodeFloatHalf) {
  g_autofree gchar* hex_string = encode_float(0.5);
  EXPECT_STREQ(hex_string, "06000000000000e03f");
}

TEST(FlStandardCodecTest, EncodeFloatFraction) {
  g_autofree gchar* hex_string = encode_float(M_PI);
  EXPECT_STREQ(hex_string, "06182d4454fb210940");
}

TEST(FlStandardCodecTest, EncodeFloatMinusZero) {
  g_autofree gchar* hex_string = encode_float(-0.0);
  EXPECT_STREQ(hex_string, "060000000000000080");
}

TEST(FlStandardCodecTest, EncodeFloatNaN) {
  g_autofree gchar* hex_string = encode_float(NAN);
  EXPECT_STREQ(hex_string, "06000000000000f87f");
}

TEST(FlStandardCodecTest, EncodeFloatInfinity) {
  g_autofree gchar* hex_string = encode_float(INFINITY);
  EXPECT_STREQ(hex_string, "06000000000000f07f");
}

TEST(FlStandardCodecTest, DecodeFloatZero) {
  g_autoptr(FlValue) value = decode_value("060000000000000000");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), 0.0);
}

TEST(FlStandardCodecTest, DecodeFloatOne) {
  g_autoptr(FlValue) value = decode_value("06000000000000f03f");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), 1.0);
}

TEST(FlStandardCodecTest, DecodeFloatMinusOne) {
  g_autoptr(FlValue) value = decode_value("06000000000000f0bf");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), -1.0);
}

TEST(FlStandardCodecTest, DecodeFloatHalf) {
  g_autoptr(FlValue) value = decode_value("06000000000000e03f");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), 0.5);
}

TEST(FlStandardCodecTest, DecodeFloatPi) {
  g_autoptr(FlValue) value = decode_value("06182d4454fb210940");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), M_PI);
}

TEST(FlStandardCodecTest, DecodeFloatMinusZero) {
  g_autoptr(FlValue) value = decode_value("060000000000000080");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), -0.0);
}

TEST(FlStandardCodecTest, DecodeFloatNaN) {
  g_autoptr(FlValue) value = decode_value("06000000000000f87f");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_TRUE(isnan(fl_value_get_float(value)));
}

TEST(FlStandardCodecTest, DecodeFloatInfinity) {
  g_autoptr(FlValue) value = decode_value("06000000000000f07f");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_TRUE(isinf(fl_value_get_float(value)));
}

TEST(FlStandardCodecTest, DecodeFloatNoData) {
  decode_error_value("0600", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeFloatShortData1) {
  decode_error_value("0600", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeFloatShortData2) {
  decode_error_value("0600000000000000", FL_CODEC_ERROR,
                     FL_CODEC_ERROR_OUT_OF_DATA);
}

static gchar* encode_string(const gchar* value) {
  g_autoptr(FlValue) v = fl_value_string_new(value);
  return encode_value(v);
}

TEST(FlStandardCodecTest, EncodeStringEmpty) {
  g_autofree gchar* hex_string = encode_string("");
  EXPECT_STREQ(hex_string, "0700");
}

TEST(FlStandardCodecTest, EncodeStringHello) {
  g_autofree gchar* hex_string = encode_string("hello");
  EXPECT_STREQ(hex_string, "070568656c6c6f");
}

TEST(FlStandardCodecTest, EncodeStringEmptySized) {
  g_autoptr(FlValue) value = fl_value_string_new_sized(NULL, 0);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "0700");
}

TEST(FlStandardCodecTest, EncodeStringHelloSized) {
  g_autoptr(FlValue) value = fl_value_string_new_sized("Hello World", 5);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "070548656c6c6f");
}

TEST(FlStandardCodecTest, DecodeStringEmpty) {
  g_autoptr(FlValue) value = decode_value("0700");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "");
}

TEST(FlStandardCodecTest, DecodeStringHello) {
  g_autoptr(FlValue) value = decode_value("070568656c6c6f");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "hello");
}

TEST(FlStandardCodecTest, DecodeStringNoData) {
  decode_error_value("07", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeStringLengthNoData) {
  decode_error_value("0705", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeStringShortData1) {
  decode_error_value("070568", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeStringShortData2) {
  decode_error_value("070568656c6c", FL_CODEC_ERROR,
                     FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, EncodeUint8ListEmpty) {
  g_autoptr(FlValue) value = fl_value_uint8_list_new(NULL, 0);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "0800");
}

TEST(FlStandardCodecTest, EncodeUint8List) {
  uint8_t data[] = {0, 1, 2, 3, 4};
  g_autoptr(FlValue) value = fl_value_uint8_list_new(data, 5);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "08050001020304");
}

TEST(FlStandardCodecTest, DecodeUint8ListEmpty) {
  g_autoptr(FlValue) value = decode_value("0800");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_UINT8_LIST);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlStandardCodecTest, DecodeUint8List) {
  g_autoptr(FlValue) value = decode_value("08050001020304");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_UINT8_LIST);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(5));
  const uint8_t* data = fl_value_get_uint8_list(value);
  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], 1);
  EXPECT_EQ(data[2], 2);
  EXPECT_EQ(data[3], 3);
  EXPECT_EQ(data[4], 4);
}

TEST(FlStandardCodecTest, DecodeUint8ListNoData) {
  decode_error_value("08", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeUint8ListLengthNoData) {
  decode_error_value("0805", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeUint8ListShortData1) {
  decode_error_value("080500", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeUint8ListShortData2) {
  decode_error_value("080500010203", FL_CODEC_ERROR,
                     FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, EncodeInt32ListEmpty) {
  g_autoptr(FlValue) value = fl_value_int32_list_new(NULL, 0);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "0900");
}

TEST(FlStandardCodecTest, EncodeInt32List) {
  int32_t data[] = {0, -1, 2, -3, 4};
  g_autoptr(FlValue) value = fl_value_int32_list_new(data, 5);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "090500000000ffffffff02000000fdffffff04000000");
}

TEST(FlStandardCodecTest, DecodeInt32ListEmpty) {
  g_autoptr(FlValue) value = decode_value("0900");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT32_LIST);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlStandardCodecTest, DecodeInt32List) {
  g_autoptr(FlValue) value =
      decode_value("090500000000ffffffff02000000fdffffff04000000");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT32_LIST);
  const int32_t* data = fl_value_get_int32_list(value);
  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], -1);
  EXPECT_EQ(data[2], 2);
  EXPECT_EQ(data[3], -3);
  EXPECT_EQ(data[4], 4);
}

TEST(FlStandardCodecTest, DecodeInt32ListNoData) {
  decode_error_value("09", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeInt32ListLengthNoData) {
  decode_error_value("0905", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeInt32ListShortData1) {
  decode_error_value("090500", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeInt32ListShortData2) {
  decode_error_value("090500000000ffffffff02000000fdffffff040000",
                     FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, EncodeInt64ListEmpty) {
  g_autoptr(FlValue) value = fl_value_int64_list_new(NULL, 0);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "0a00");
}

TEST(FlStandardCodecTest, EncodeInt64List) {
  int64_t data[] = {0, -1, 2, -3, 4};
  g_autoptr(FlValue) value = fl_value_int64_list_new(data, 5);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string,
               "0a050000000000000000ffffffffffffffff0200000000000000fdffffff"
               "ffffffff0400000000000000");
}

TEST(FlStandardCodecTest, DecodeInt64ListEmpty) {
  g_autoptr(FlValue) value = decode_value("0a00");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT64_LIST);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlStandardCodecTest, DecodeInt64List) {
  g_autoptr(FlValue) value = decode_value(
      "0a050000000000000000ffffffffffffffff0200000000000000fdffffff"
      "ffffffff0400000000000000");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT64_LIST);
  const int64_t* data = fl_value_get_int64_list(value);
  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], -1);
  EXPECT_EQ(data[2], 2);
  EXPECT_EQ(data[3], -3);
  EXPECT_EQ(data[4], 4);
}

TEST(FlStandardCodecTest, DecodeInt64ListNoData) {
  decode_error_value("0a", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeInt64ListLengthNoData) {
  decode_error_value("0a05", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeInt64ListShortData1) {
  decode_error_value("0a0500", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeInt64ListShortData2) {
  decode_error_value(
      "0a050000000000000000ffffffffffffffff0200000000000000fdffffffffffffff0400"
      "0000000000",
      FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, EncodeFloatListEmpty) {
  g_autoptr(FlValue) value = fl_value_float_list_new(NULL, 0);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "0b00");
}

TEST(FlStandardCodecTest, EncodeFloatList) {
  double data[] = {0, -0.5, 0.25, -0.125, 0.00625};
  g_autoptr(FlValue) value = fl_value_float_list_new(data, 5);
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string,
               "0b050000000000000000000000000000e0bf000000000000d03f00000000"
               "0000c0bf9a9999999999793f");
}

TEST(FlStandardCodecTest, DecodeFloatListEmpty) {
  g_autoptr(FlValue) value = decode_value("0b00");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT_LIST);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlStandardCodecTest, DecodeFloatList) {
  g_autoptr(FlValue) value = decode_value(
      "0b050000000000000000000000000000e0bf000000000000d03f00000000"
      "0000c0bf9a9999999999793f");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT_LIST);
  const double* data = fl_value_get_float_list(value);
  EXPECT_FLOAT_EQ(data[0], 0.0);
  EXPECT_FLOAT_EQ(data[1], -0.5);
  EXPECT_FLOAT_EQ(data[2], 0.25);
  EXPECT_FLOAT_EQ(data[3], -0.125);
  EXPECT_FLOAT_EQ(data[4], 0.00625);
}

TEST(FlStandardCodecTest, DecodeFloatListNoData) {
  decode_error_value("0b", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeFloatListLengthNoData) {
  decode_error_value("0b05", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeFloatListShortData1) {
  decode_error_value("0b0500", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeFloatListShortData2) {
  decode_error_value(
      "0b050000000000000000000000000000e0bf000000000000d03f000000000000c0bf9a99"
      "9999999979",
      FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, EncodeListEmpty) {
  g_autoptr(FlValue) value = fl_value_list_new();
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "0c00");
}

TEST(FlStandardCodecTest, EncodeListTypes) {
  g_autoptr(FlValue) value = fl_value_list_new();
  fl_value_list_add_take(value, fl_value_null_new());
  fl_value_list_add_take(value, fl_value_bool_new(TRUE));
  fl_value_list_add_take(value, fl_value_int_new(42));
  fl_value_list_add_take(value, fl_value_float_new(-1.5));
  fl_value_list_add_take(value, fl_value_string_new("hello"));
  fl_value_list_add_take(value, fl_value_list_new());
  fl_value_list_add_take(value, fl_value_map_new());
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string,
               "0c070001032a00000006000000000000f8bf070568656c6c6f0c000d00");
}

TEST(FlStandardCodecTest, EncodeListNested) {
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
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string,
               "0c020c05030000000003020000000304000000030600000003080000000c"
               "0503010000000303000000030500000003070000000309000000");
}

TEST(FlStandardCodecTest, DecodeListEmpty) {
  g_autoptr(FlValue) value = decode_value("0c00");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_LIST);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlStandardCodecTest, DecodeListTypes) {
  g_autoptr(FlValue) value = decode_value(
      "0c070001032a00000006000000000000f8bf070568656c6c6f0c000d00");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_LIST);
  ASSERT_EQ(fl_value_get_length(value), static_cast<size_t>(7));
  ASSERT_EQ(fl_value_get_type(fl_value_list_get_value(value, 0)),
            FL_VALUE_TYPE_NULL);
  ASSERT_EQ(fl_value_get_type(fl_value_list_get_value(value, 1)),
            FL_VALUE_TYPE_BOOL);
  EXPECT_TRUE(fl_value_get_bool(fl_value_list_get_value(value, 1)));
  ASSERT_EQ(fl_value_get_type(fl_value_list_get_value(value, 2)),
            FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(fl_value_list_get_value(value, 2)), 42);
  ASSERT_EQ(fl_value_get_type(fl_value_list_get_value(value, 3)),
            FL_VALUE_TYPE_FLOAT);
  EXPECT_FLOAT_EQ(fl_value_get_float(fl_value_list_get_value(value, 3)), -1.5);
  ASSERT_EQ(fl_value_get_type(fl_value_list_get_value(value, 4)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_list_get_value(value, 4)), "hello");
  ASSERT_EQ(fl_value_get_type(fl_value_list_get_value(value, 5)),
            FL_VALUE_TYPE_LIST);
  ASSERT_EQ(fl_value_get_length(fl_value_list_get_value(value, 5)),
            static_cast<size_t>(0));
  ASSERT_EQ(fl_value_get_type(fl_value_list_get_value(value, 6)),
            FL_VALUE_TYPE_MAP);
  ASSERT_EQ(fl_value_get_length(fl_value_list_get_value(value, 6)),
            static_cast<size_t>(0));
}

TEST(FlStandardCodecTest, DecodeListNested) {
  g_autoptr(FlValue) value = decode_value(
      "0c020c05030000000003020000000304000000030600000003080000000c"
      "0503010000000303000000030500000003070000000309000000");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_LIST);
  ASSERT_EQ(fl_value_get_length(value), static_cast<size_t>(2));
  FlValue* even_list = fl_value_list_get_value(value, 0);
  ASSERT_EQ(fl_value_get_type(even_list), FL_VALUE_TYPE_LIST);
  ASSERT_EQ(fl_value_get_length(even_list), static_cast<size_t>(5));
  FlValue* odd_list = fl_value_list_get_value(value, 1);
  ASSERT_EQ(fl_value_get_type(odd_list), FL_VALUE_TYPE_LIST);
  ASSERT_EQ(fl_value_get_length(odd_list), static_cast<size_t>(5));
  for (int i = 0; i < 5; i++) {
    FlValue* v = fl_value_list_get_value(even_list, i);
    ASSERT_EQ(fl_value_get_type(v), FL_VALUE_TYPE_INT);
    EXPECT_EQ(fl_value_get_int(v), i * 2);

    v = fl_value_list_get_value(odd_list, i);
    ASSERT_EQ(fl_value_get_type(v), FL_VALUE_TYPE_INT);
    EXPECT_EQ(fl_value_get_int(v), i * 2 + 1);
  }
}

TEST(FlStandardCodecTest, DecodeListNoData) {
  decode_error_value("0c", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeListLengthNoData) {
  decode_error_value("0c07", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeListShortData1) {
  decode_error_value("0c0700", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeListShortData2) {
  decode_error_value("0c070001032a00000006000000000000f8bf070568656c6c6f0c000d",
                     FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, EncodeMapEmpty) {
  g_autoptr(FlValue) value = fl_value_map_new();
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string, "0d00");
}

TEST(FlStandardCodecTest, EncodeMapKeyTypes) {
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
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string,
               "0d070007046e756c6c010704626f6f6c032a0000000703696e7406000000"
               "000000f8bf0705666c6f6174070568656c6c6f0706737472696e670c0007"
               "046c6973740d0007036d6170");
}

TEST(FlStandardCodecTest, EncodeMapValueTypes) {
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
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string,
               "0d0707046e756c6c000704626f6f6c010703696e74032a0000000705666c"
               "6f617406000000000000f8bf0706737472696e67070568656c6c6f07046c"
               "6973740c0007036d61700d00");
}

TEST(FlStandardCodecTest, EncodeMapNested) {
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
  g_autofree gchar* hex_string = encode_value(value);
  EXPECT_STREQ(hex_string,
               "0d02070a7374722d746f2d696e740d0407047a65726f030000000007036f"
               "6e650301000000070374776f030200000007057468726565030300000007"
               "0a696e742d746f2d7374720d04030000000007047a65726f030100000007"
               "036f6e650302000000070374776f030300000007057468726565");
}

TEST(FlStandardCodecTest, DecodeMapEmpty) {
  g_autoptr(FlValue) value = decode_value("0d00");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_MAP);
  ASSERT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlStandardCodecTest, DecodeMapKeyTypes) {
  g_autoptr(FlValue) value = decode_value(
      "0d070007046e756c6c010704626f6f6c032a0000000703696e7406000000"
      "000000f8bf0705666c6f6174070568656c6c6f0706737472696e670c0007"
      "046c6973740d0007036d6170");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_MAP);
  ASSERT_EQ(fl_value_get_length(value), static_cast<size_t>(7));

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 0)),
            FL_VALUE_TYPE_NULL);
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 0)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_value(value, 0)), "null");

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 1)),
            FL_VALUE_TYPE_BOOL);
  EXPECT_TRUE(fl_value_get_bool(fl_value_map_get_key(value, 1)));
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 1)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_value(value, 1)), "bool");

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 2)),
            FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(fl_value_map_get_key(value, 2)), 42);
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 2)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_value(value, 2)), "int");

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 3)),
            FL_VALUE_TYPE_FLOAT);
  EXPECT_FLOAT_EQ(fl_value_get_float(fl_value_map_get_key(value, 3)), -1.5);
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 3)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_value(value, 3)), "float");

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 4)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 4)), "hello");
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 4)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_value(value, 4)), "string");

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 5)),
            FL_VALUE_TYPE_LIST);
  ASSERT_EQ(fl_value_get_length(fl_value_map_get_key(value, 5)),
            static_cast<size_t>(0));
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 5)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_value(value, 5)), "list");

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 6)),
            FL_VALUE_TYPE_MAP);
  ASSERT_EQ(fl_value_get_length(fl_value_map_get_key(value, 6)),
            static_cast<size_t>(0));
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 6)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_value(value, 6)), "map");
}

TEST(FlStandardCodecTest, DecodeMapValueTypes) {
  g_autoptr(FlValue) value = decode_value(
      "0d0707046e756c6c000704626f6f6c010703696e74032a0000000705666c"
      "6f617406000000000000f8bf0706737472696e67070568656c6c6f07046c"
      "6973740c0007036d61700d00");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_MAP);
  ASSERT_EQ(fl_value_get_length(value), static_cast<size_t>(7));

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 0)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 0)), "null");
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 0)),
            FL_VALUE_TYPE_NULL);

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 1)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 1)), "bool");
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 1)),
            FL_VALUE_TYPE_BOOL);
  EXPECT_TRUE(fl_value_get_bool(fl_value_map_get_value(value, 1)));

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 2)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 2)), "int");
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 2)),
            FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(fl_value_map_get_value(value, 2)), 42);

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 3)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 3)), "float");
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 3)),
            FL_VALUE_TYPE_FLOAT);
  EXPECT_FLOAT_EQ(fl_value_get_float(fl_value_map_get_value(value, 3)), -1.5);

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 4)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 4)), "string");
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 4)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_value(value, 4)), "hello");

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 5)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 5)), "list");
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 5)),
            FL_VALUE_TYPE_LIST);
  ASSERT_EQ(fl_value_get_length(fl_value_map_get_value(value, 5)),
            static_cast<size_t>(0));

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 6)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 6)), "map");
  ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(value, 6)),
            FL_VALUE_TYPE_MAP);
  ASSERT_EQ(fl_value_get_length(fl_value_map_get_value(value, 6)),
            static_cast<size_t>(0));
}

TEST(FlStandardCodecTest, DecodeMapNested) {
  g_autoptr(FlValue) value = decode_value(
      "0d02070a7374722d746f2d696e740d0407047a65726f030000000007036f"
      "6e650301000000070374776f030200000007057468726565030300000007"
      "0a696e742d746f2d7374720d04030000000007047a65726f030100000007"
      "036f6e650302000000070374776f030300000007057468726565");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_MAP);
  ASSERT_EQ(fl_value_get_length(value), static_cast<size_t>(2));

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 0)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 0)),
               "str-to-int");
  FlValue* str_to_int = fl_value_map_get_value(value, 0);
  ASSERT_EQ(fl_value_get_type(str_to_int), FL_VALUE_TYPE_MAP);
  ASSERT_EQ(fl_value_get_length(str_to_int), static_cast<size_t>(4));

  ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(value, 1)),
            FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(value, 1)),
               "int-to-str");
  FlValue* int_to_str = fl_value_map_get_value(value, 1);
  ASSERT_EQ(fl_value_get_type(int_to_str), FL_VALUE_TYPE_MAP);
  ASSERT_EQ(fl_value_get_length(int_to_str), static_cast<size_t>(4));

  const char* numbers[] = {"zero", "one", "two", "three", NULL};
  for (int i = 0; numbers[i] != NULL; i++) {
    ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(str_to_int, i)),
              FL_VALUE_TYPE_STRING);
    EXPECT_STREQ(fl_value_get_string(fl_value_map_get_key(str_to_int, i)),
                 numbers[i]);

    ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(str_to_int, i)),
              FL_VALUE_TYPE_INT);
    EXPECT_EQ(fl_value_get_int(fl_value_map_get_value(str_to_int, i)), i);

    ASSERT_EQ(fl_value_get_type(fl_value_map_get_key(int_to_str, i)),
              FL_VALUE_TYPE_INT);
    EXPECT_EQ(fl_value_get_int(fl_value_map_get_key(int_to_str, i)), i);

    ASSERT_EQ(fl_value_get_type(fl_value_map_get_value(int_to_str, i)),
              FL_VALUE_TYPE_STRING);
    EXPECT_STREQ(fl_value_get_string(fl_value_map_get_value(int_to_str, i)),
                 numbers[i]);
  }
}

TEST(FlStandardCodecTest, DecodeMapNoData) {
  decode_error_value("0d", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeMapLengthNoData) {
  decode_error_value("0d07", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeMapShortData1) {
  decode_error_value("0d0707", FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeMapShortData2) {
  decode_error_value(
      "0d0707046e756c6c000704626f6f6c010703696e74032a0000000705666c"
      "6f617406000000000000f8bf0706737472696e67070568656c6c6f07046c"
      "6973740c0007036d61700d",
      FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlStandardCodecTest, DecodeUnknownType) {
  decode_error_value("0e", FL_STANDARD_CODEC_ERROR,
                     FL_STANDARD_CODEC_ERROR_UNKNOWN_TYPE);
}
