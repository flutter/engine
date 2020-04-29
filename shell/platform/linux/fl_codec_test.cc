// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_codec.h"
#include "gtest/gtest.h"

G_DECLARE_FINAL_TYPE(FlTestCodec, fl_test_codec, FL, TEST_CODEC, FlCodec)

struct _FlTestCodec {
  FlCodec parent_instance;
};

G_DEFINE_TYPE(FlTestCodec, fl_test_codec, fl_codec_get_type())

static gboolean fl_test_codec_write_value(FlCodec* codec,
                                          GByteArray* buffer,
                                          FlValue* value,
                                          GError** error) {
  EXPECT_TRUE(FL_IS_TEST_CODEC(codec));

  if (fl_value_get_type(value) == FL_VALUE_TYPE_INT) {
    char c = '0' + fl_value_get_int(value);
    g_byte_array_append(buffer, reinterpret_cast<const guint8*>(&c), 1);
    return TRUE;
  } else {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED, "ERROR");
    return FALSE;
  }
}

static FlValue* fl_test_codec_read_value(FlCodec* codec,
                                         GBytes* message,
                                         size_t* offset,
                                         GError** error) {
  EXPECT_TRUE(FL_IS_TEST_CODEC(codec));
  EXPECT_TRUE(*offset < g_bytes_get_size(message));

  size_t data_length;
  const uint8_t* data =
      static_cast<const uint8_t*>(g_bytes_get_data(message, &data_length));
  if (data_length == 0) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED, "ERROR");
    return FALSE;
  }

  g_autoptr(FlValue) value = fl_value_int_new(data[*offset] - '0');
  (*offset)++;
  return fl_value_ref(value);
}

static void fl_test_codec_class_init(FlTestCodecClass* klass) {
  FL_CODEC_CLASS(klass)->write_value = fl_test_codec_write_value;
  FL_CODEC_CLASS(klass)->read_value = fl_test_codec_read_value;
}

static void fl_test_codec_init(FlTestCodec* self) {}

static FlTestCodec* fl_test_codec_new() {
  return FL_TEST_CODEC(g_object_new(fl_test_codec_get_type(), nullptr));
}

TEST(FlCodecTest, WriteValue) {
  g_autoptr(FlTestCodec) codec = fl_test_codec_new();
  g_autoptr(GByteArray) buffer = g_byte_array_new();

  g_autoptr(FlValue) value = fl_value_int_new(1);
  g_autoptr(GError) error = nullptr;
  gboolean result =
      fl_codec_write_value(FL_CODEC(codec), buffer, value, &error);
  EXPECT_TRUE(result);
  EXPECT_EQ(error, nullptr);
  EXPECT_EQ(buffer->len, static_cast<unsigned int>(1));
  EXPECT_EQ(buffer->data[0], '1');
}

TEST(FlCodecTest, WriteValues) {
  g_autoptr(FlTestCodec) codec = fl_test_codec_new();
  g_autoptr(GByteArray) buffer = g_byte_array_new();

  for (int i = 1; i <= 5; i++) {
    g_autoptr(FlValue) value = fl_value_int_new(i);
    g_autoptr(GError) error = nullptr;
    gboolean result =
        fl_codec_write_value(FL_CODEC(codec), buffer, value, &error);
    EXPECT_TRUE(result);
    EXPECT_EQ(error, nullptr);
  }

  EXPECT_EQ(buffer->len, static_cast<unsigned int>(5));
  for (int i = 1; i <= 5; i++)
    EXPECT_EQ(buffer->data[i - 1], '0' + i);
}

TEST(FlCodecTest, WriteValueError) {
  g_autoptr(FlTestCodec) codec = fl_test_codec_new();
  g_autoptr(GByteArray) buffer = g_byte_array_new();

  g_autoptr(FlValue) value = fl_value_null_new();
  g_autoptr(GError) error = nullptr;
  gboolean result =
      fl_codec_write_value(FL_CODEC(codec), buffer, value, &error);
  EXPECT_FALSE(result);
  EXPECT_TRUE(g_error_matches(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED));
  EXPECT_EQ(buffer->len, static_cast<unsigned int>(0));
}

TEST(FlCodecTest, ReadValueEmpty) {
  g_autoptr(FlTestCodec) codec = fl_test_codec_new();
  g_autoptr(GBytes) message = g_bytes_new(nullptr, 0);

  size_t offset = 0;
  g_autoptr(GError) error = nullptr;
  g_autoptr(FlValue) value =
      fl_codec_read_value(FL_CODEC(codec), message, &offset, &error);
  EXPECT_EQ(value, nullptr);
  EXPECT_TRUE(
      g_error_matches(error, FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA));
  EXPECT_EQ(offset, static_cast<size_t>(0));
}

TEST(FlCodecTest, ReadValue) {
  g_autoptr(FlTestCodec) codec = fl_test_codec_new();
  uint8_t data[] = {'1'};
  g_autoptr(GBytes) message = g_bytes_new(data, 1);

  size_t offset = 0;
  g_autoptr(GError) error = nullptr;
  g_autoptr(FlValue) value =
      fl_codec_read_value(FL_CODEC(codec), message, &offset, &error);
  EXPECT_NE(value, nullptr);
  EXPECT_EQ(error, nullptr);
  EXPECT_EQ(offset, static_cast<size_t>(1));

  ASSERT_TRUE(fl_value_get_type(value) == FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 1);
}

TEST(FlCodecTest, ReadValues) {
  g_autoptr(FlTestCodec) codec = fl_test_codec_new();
  uint8_t data[] = {'1', '2', '3', '4', '5'};
  g_autoptr(GBytes) message = g_bytes_new(data, 5);

  size_t offset = 0;
  for (int i = 1; i <= 5; i++) {
    g_autoptr(GError) error = nullptr;
    g_autoptr(FlValue) value =
        fl_codec_read_value(FL_CODEC(codec), message, &offset, &error);
    EXPECT_NE(value, nullptr);
    EXPECT_EQ(error, nullptr);
    ASSERT_TRUE(fl_value_get_type(value) == FL_VALUE_TYPE_INT);
    EXPECT_EQ(fl_value_get_int(value), i);
  }

  EXPECT_EQ(offset, static_cast<size_t>(5));
}

TEST(FlCodecTest, ReadValueNullOffset) {
  g_autoptr(FlTestCodec) codec = fl_test_codec_new();
  uint8_t data[] = {'1'};
  g_autoptr(GBytes) message = g_bytes_new(data, 1);

  g_autoptr(GError) error = nullptr;
  g_autoptr(FlValue) value =
      fl_codec_read_value(FL_CODEC(codec), message, NULL, &error);
  EXPECT_NE(value, nullptr);
  EXPECT_EQ(error, nullptr);

  ASSERT_TRUE(fl_value_get_type(value) == FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 1);
}

TEST(FlCodecTest, ReadValueInvalidOffset) {
  g_autoptr(FlTestCodec) codec = fl_test_codec_new();
  uint8_t data[] = {'1', '2', '3', '4', '5'};
  g_autoptr(GBytes) message = g_bytes_new(data, 5);

  size_t offset = 9999;
  g_autoptr(GError) error = nullptr;
  g_autoptr(FlValue) value =
      fl_codec_read_value(FL_CODEC(codec), message, &offset, &error);
  EXPECT_EQ(value, nullptr);
  EXPECT_TRUE(
      g_error_matches(error, FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA));
  EXPECT_EQ(offset, static_cast<size_t>(9999));
}
