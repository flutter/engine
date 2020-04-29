// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_codec.h"

#include <gmodule.h>

G_DEFINE_QUARK(fl_standard_codec_error_quark, fl_standard_codec_error)

// See lib/src/services/message_codecs.dart in Flutter source for description of
// encoding

// Type values
static constexpr int kValueNull = 0;
static constexpr int kValueTrue = 1;
static constexpr int kValueFalse = 2;
static constexpr int kValueInt32 = 3;
static constexpr int kValueInt64 = 4;
static constexpr int kValueFloat64 = 6;
static constexpr int kValueString = 7;
static constexpr int kValueUint8List = 8;
static constexpr int kValueInt32List = 9;
static constexpr int kValueInt64List = 10;
static constexpr int kValueFloat64List = 11;
static constexpr int kValueList = 12;
static constexpr int kValueMap = 13;

// TODO(robert-ancell) Add support for large integer type (kValueLargeInt = 5)

struct _FlStandardCodec {
  FlCodec parent_instance;
};

G_DEFINE_TYPE(FlStandardCodec, fl_standard_codec, fl_codec_get_type())

static void write_uint8(GByteArray* buffer, uint8_t value) {
  g_byte_array_append(buffer, &value, 1);
}

static void write_uint16(GByteArray* buffer, uint16_t value) {
  g_byte_array_append(buffer, reinterpret_cast<uint8_t*>(&value),
                      sizeof(uint16_t));
}

static void write_uint32(GByteArray* buffer, uint32_t value) {
  g_byte_array_append(buffer, reinterpret_cast<uint8_t*>(&value),
                      sizeof(uint32_t));
}

static void write_int32(GByteArray* buffer, int32_t value) {
  g_byte_array_append(buffer, reinterpret_cast<uint8_t*>(&value),
                      sizeof(int32_t));
}

static void write_int64(GByteArray* buffer, int64_t value) {
  g_byte_array_append(buffer, reinterpret_cast<uint8_t*>(&value),
                      sizeof(int64_t));
}

static void write_float64(GByteArray* buffer, double value) {
  g_byte_array_append(buffer, reinterpret_cast<uint8_t*>(&value),
                      sizeof(double));
}

static void write_size(GByteArray* buffer, uint32_t size) {
  if (size < 254)
    write_uint8(buffer, size);
  else if (size <= 0xffff) {
    write_uint8(buffer, 254);
    write_uint16(buffer, size);
  } else {
    write_uint8(buffer, 255);
    write_uint32(buffer, size);
  }
}

static void encode_value(GByteArray* buffer, FlValue* value) {
  if (value == nullptr) {
    write_uint8(buffer, kValueNull);
    return;
  }

  switch (fl_value_get_type(value)) {
    case FL_VALUE_TYPE_NULL:
      write_uint8(buffer, kValueNull);
      break;
    case FL_VALUE_TYPE_BOOL:
      if (fl_value_get_bool(value))
        write_uint8(buffer, kValueTrue);
      else
        write_uint8(buffer, kValueFalse);
      break;
    case FL_VALUE_TYPE_INT: {
      int64_t v = fl_value_get_int(value);
      if (v >= INT32_MIN && v <= INT32_MAX) {
        write_uint8(buffer, kValueInt32);
        write_int32(buffer, v);
      } else {
        write_uint8(buffer, kValueInt64);
        write_int64(buffer, v);
      }
      break;
    }
    case FL_VALUE_TYPE_FLOAT:
      write_uint8(buffer, kValueFloat64);
      write_float64(buffer, fl_value_get_float(value));
      break;
    case FL_VALUE_TYPE_STRING: {
      write_uint8(buffer, kValueString);
      const char* text = fl_value_get_string(value);
      size_t length = strlen(text);
      write_size(buffer, length);
      g_byte_array_append(buffer, reinterpret_cast<const uint8_t*>(text),
                          length);
      break;
    }
    case FL_VALUE_TYPE_UINT8_LIST: {
      write_uint8(buffer, kValueUint8List);
      size_t length = fl_value_get_length(value);
      write_size(buffer, length);
      g_byte_array_append(buffer, fl_value_get_uint8_list(value),
                          sizeof(uint8_t) * length);
      break;
    }
    case FL_VALUE_TYPE_INT32_LIST: {
      write_uint8(buffer, kValueInt32List);
      size_t length = fl_value_get_length(value);
      write_size(buffer, length);
      g_byte_array_append(
          buffer,
          reinterpret_cast<const uint8_t*>(fl_value_get_int32_list(value)),
          sizeof(int32_t) * length);
      break;
    }
    case FL_VALUE_TYPE_INT64_LIST: {
      write_uint8(buffer, kValueInt64List);
      size_t length = fl_value_get_length(value);
      write_size(buffer, length);
      g_byte_array_append(
          buffer,
          reinterpret_cast<const uint8_t*>(fl_value_get_int64_list(value)),
          sizeof(int64_t) * length);
      break;
    }
    case FL_VALUE_TYPE_FLOAT_LIST: {
      write_uint8(buffer, kValueFloat64List);
      size_t length = fl_value_get_length(value);
      write_size(buffer, length);
      g_byte_array_append(
          buffer,
          reinterpret_cast<const uint8_t*>(fl_value_get_float_list(value)),
          sizeof(double) * length);
      break;
    }
    case FL_VALUE_TYPE_LIST:
      write_uint8(buffer, kValueList);
      write_size(buffer, fl_value_get_length(value));
      for (size_t i = 0; i < fl_value_get_length(value); i++)
        encode_value(buffer, fl_value_list_get_value(value, i));
      break;
    case FL_VALUE_TYPE_MAP:
      write_uint8(buffer, kValueMap);
      write_size(buffer, fl_value_get_length(value));
      for (size_t i = 0; i < fl_value_get_length(value); i++) {
        encode_value(buffer, fl_value_map_get_key(value, i));
        encode_value(buffer, fl_value_map_get_value(value, i));
      }
      break;
  }
}

static gboolean check_size(size_t data_length,
                           size_t* offset,
                           size_t required,
                           GError** error) {
  if (*offset + required > data_length) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA,
                "Unexpected end of data");
    return FALSE;
  }
  return TRUE;
}

static gboolean read_uint8(const uint8_t* data,
                           size_t data_length,
                           size_t* offset,
                           uint8_t* value,
                           GError** error) {
  if (!check_size(data_length, offset, 1, error))
    return FALSE;

  *value = data[*offset];
  (*offset)++;
  return TRUE;
}

static gboolean read_uint16(const uint8_t* data,
                            size_t data_length,
                            size_t* offset,
                            uint16_t* value,
                            GError** error) {
  if (!check_size(data_length, offset, 2, error))
    return FALSE;

  *value = *reinterpret_cast<const uint16_t*>(data + *offset);
  *offset += 2;
  return TRUE;
}

static gboolean read_uint32(const uint8_t* data,
                            size_t data_length,
                            size_t* offset,
                            uint32_t* value,
                            GError** error) {
  if (!check_size(data_length, offset, 4, error))
    return FALSE;

  *value = *reinterpret_cast<const uint32_t*>(data + *offset);
  *offset += 4;
  return TRUE;
}

static gboolean read_size(const uint8_t* data,
                          size_t data_length,
                          size_t* offset,
                          uint32_t* value,
                          GError** error) {
  uint8_t value8;
  if (!read_uint8(data, data_length, offset, &value8, error))
    return FALSE;

  if (value8 == 255) {
    if (!read_uint32(data, data_length, offset, value, error))
      return FALSE;
  } else if (value8 == 254) {
    uint16_t value16;
    if (!read_uint16(data, data_length, offset, &value16, error))
      return FALSE;
    *value = value16;
  } else
    *value = value8;

  return TRUE;
}

static FlValue* decode_int32(const uint8_t* data,
                             size_t data_length,
                             size_t* offset,
                             GError** error) {
  if (!check_size(data_length, offset, 4, error))
    return nullptr;

  FlValue* value =
      fl_value_int_new(*reinterpret_cast<const int32_t*>(data + *offset));
  *offset += 4;
  return value;
}

static FlValue* decode_int64(const uint8_t* data,
                             size_t data_length,
                             size_t* offset,
                             GError** error) {
  if (!check_size(data_length, offset, 8, error))
    return nullptr;

  FlValue* value =
      fl_value_int_new(*reinterpret_cast<const int64_t*>(data + *offset));
  *offset += 8;
  return value;
}

static FlValue* decode_float64(const uint8_t* data,
                               size_t data_length,
                               size_t* offset,
                               GError** error) {
  if (!check_size(data_length, offset, 8, error))
    return nullptr;

  FlValue* value =
      fl_value_float_new(*reinterpret_cast<const double*>(data + *offset));
  *offset += 8;
  return value;
}

static FlValue* decode_string(const uint8_t* data,
                              size_t data_length,
                              size_t* offset,
                              GError** error) {
  uint32_t length;
  if (!read_size(data, data_length, offset, &length, error))
    return nullptr;
  if (!check_size(data_length, offset, length, error))
    return nullptr;
  FlValue* value = fl_value_string_new_sized(
      reinterpret_cast<const gchar*>(data + *offset), length);
  *offset += length;
  return value;
}

static FlValue* decode_uint8_list(const uint8_t* data,
                                  size_t data_length,
                                  size_t* offset,
                                  GError** error) {
  uint32_t length;
  if (!read_size(data, data_length, offset, &length, error))
    return nullptr;
  if (!check_size(data_length, offset, sizeof(uint8_t) * length, error))
    return nullptr;
  FlValue* value = fl_value_uint8_list_new(data + *offset, length);
  *offset += length;
  return value;
}

static FlValue* decode_int32_list(const uint8_t* data,
                                  size_t data_length,
                                  size_t* offset,
                                  GError** error) {
  uint32_t length;
  if (!read_size(data, data_length, offset, &length, error))
    return nullptr;
  if (!check_size(data_length, offset, sizeof(int32_t) * length, error))
    return nullptr;
  FlValue* value = fl_value_int32_list_new(
      reinterpret_cast<const int32_t*>(data + *offset), length);
  *offset += sizeof(int32_t) * length;
  return value;
}

static FlValue* decode_int64_list(const uint8_t* data,
                                  size_t data_length,
                                  size_t* offset,
                                  GError** error) {
  uint32_t length;
  if (!read_size(data, data_length, offset, &length, error))
    return nullptr;
  if (!check_size(data_length, offset, sizeof(int64_t) * length, error))
    return nullptr;
  FlValue* value = fl_value_int64_list_new(
      reinterpret_cast<const int64_t*>(data + *offset), length);
  *offset += sizeof(int64_t) * length;
  return value;
}

static FlValue* decode_float64_list(const uint8_t* data,
                                    size_t data_length,
                                    size_t* offset,
                                    GError** error) {
  uint32_t length;
  if (!read_size(data, data_length, offset, &length, error))
    return nullptr;
  if (!check_size(data_length, offset, sizeof(double) * length, error))
    return nullptr;
  FlValue* value = fl_value_float_list_new(
      reinterpret_cast<const double*>(data + *offset), length);
  *offset += sizeof(double) * length;
  return value;
}

static FlValue* decode_value(const uint8_t* data,
                             size_t data_length,
                             size_t* offset,
                             GError** error);

static FlValue* decode_list(const uint8_t* data,
                            size_t data_length,
                            size_t* offset,
                            GError** error) {
  uint32_t length;
  if (!read_size(data, data_length, offset, &length, error))
    return nullptr;

  g_autoptr(FlValue) list = fl_value_list_new();
  for (size_t i = 0; i < length; i++) {
    g_autoptr(FlValue) child = decode_value(data, data_length, offset, error);
    if (child == nullptr)
      return nullptr;
    fl_value_list_add(list, child);
  }

  return fl_value_ref(list);
}

static FlValue* decode_map(const uint8_t* data,
                           size_t data_length,
                           size_t* offset,
                           GError** error) {
  uint32_t length;
  if (!read_size(data, data_length, offset, &length, error))
    return nullptr;

  g_autoptr(FlValue) map = fl_value_map_new();
  for (size_t i = 0; i < length; i++) {
    g_autoptr(FlValue) key = decode_value(data, data_length, offset, error);
    if (key == nullptr)
      return nullptr;
    g_autoptr(FlValue) value = decode_value(data, data_length, offset, error);
    if (value == nullptr)
      return nullptr;
    fl_value_map_set(map, key, value);
  }

  return fl_value_ref(map);
}

static FlValue* decode_value(const uint8_t* data,
                             size_t data_length,
                             size_t* offset,
                             GError** error) {
  uint8_t type;
  if (!read_uint8(data, data_length, offset, &type, error))
    return nullptr;

  g_autoptr(FlValue) value = nullptr;
  if (type == kValueNull)
    return fl_value_null_new();
  else if (type == kValueTrue)
    return fl_value_bool_new(TRUE);
  else if (type == kValueFalse)
    return fl_value_bool_new(FALSE);
  else if (type == kValueInt32)
    value = decode_int32(data, data_length, offset, error);
  else if (type == kValueInt64)
    value = decode_int64(data, data_length, offset, error);
  else if (type == kValueFloat64)
    value = decode_float64(data, data_length, offset, error);
  else if (type == kValueString)
    value = decode_string(data, data_length, offset, error);
  else if (type == kValueUint8List)
    value = decode_uint8_list(data, data_length, offset, error);
  else if (type == kValueInt32List)
    value = decode_int32_list(data, data_length, offset, error);
  else if (type == kValueInt64List)
    value = decode_int64_list(data, data_length, offset, error);
  else if (type == kValueFloat64List)
    value = decode_float64_list(data, data_length, offset, error);
  else if (type == kValueList)
    value = decode_list(data, data_length, offset, error);
  else if (type == kValueMap)
    value = decode_map(data, data_length, offset, error);
  else {
    g_set_error(error, FL_STANDARD_CODEC_ERROR,
                FL_STANDARD_CODEC_ERROR_UNKNOWN_TYPE,
                "Unexpected standard codec type %02x", type);
    return nullptr;
  }

  return value == nullptr ? nullptr : fl_value_ref(value);
}

static gboolean fl_standard_codec_write_value(FlCodec* codec,
                                              GByteArray* buffer,
                                              FlValue* value,
                                              GError** error) {
  encode_value(buffer, value);
  return TRUE;
}

static FlValue* fl_standard_codec_read_value(FlCodec* codec,
                                             GBytes* message,
                                             size_t* offset,
                                             GError** error) {
  gsize data_length;
  const uint8_t* data =
      static_cast<const uint8_t*>(g_bytes_get_data(message, &data_length));
  g_autoptr(FlValue) value = decode_value(data, data_length, offset, error);

  return value == nullptr ? nullptr : fl_value_ref(value);
}

static void fl_standard_codec_class_init(FlStandardCodecClass* klass) {
  FL_CODEC_CLASS(klass)->write_value = fl_standard_codec_write_value;
  FL_CODEC_CLASS(klass)->read_value = fl_standard_codec_read_value;
}

static void fl_standard_codec_init(FlStandardCodec* self) {}

G_MODULE_EXPORT FlStandardCodec* fl_standard_codec_new() {
  return static_cast<FlStandardCodec*>(
      g_object_new(fl_standard_codec_get_type(), nullptr));
}
