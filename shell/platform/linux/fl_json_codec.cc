// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_json_codec.h"

#include <gmodule.h>

G_DEFINE_QUARK(fl_json_codec_error_quark, fl_json_codec_error)

// JSON spec is at https://www.json.org/json-en.html

struct _FlJsonCodec {
  FlCodec parent_instance;
};

G_DEFINE_TYPE(FlJsonCodec, fl_json_codec, fl_codec_get_type())

static void write_char(GByteArray* buffer, gchar value) {
  const guint8 v = value;
  g_byte_array_append(buffer, &v, 1);
}

static void write_string(GByteArray* buffer, const gchar* value) {
  g_byte_array_append(buffer, reinterpret_cast<const guint8*>(value),
                      strlen(value));
}

static char json_int_to_digit(int value) {
  return '0' + value;
}

static void write_int(GByteArray* buffer, int64_t value) {
  // Special case the minimum value as it can't be inverted and fit in a signed
  // 64 bit value
  if (value == G_MININT64) {
    write_string(buffer, "-9223372036854775808");
    return;
  }

  if (value < 0) {
    write_char(buffer, '-');
    value = -value;
  }

  int64_t divisor = 1;
  while (value / divisor > 9)
    divisor *= 10;

  while (TRUE) {
    int64_t v = value / divisor;
    write_char(buffer, json_int_to_digit(v));
    if (divisor == 1)
      return;
    value -= v * divisor;
    divisor /= 10;
  }
}

static gboolean write_double(GByteArray* buffer, double value, GError** error) {
  if (!isfinite(value)) {
    g_set_error(error, FL_JSON_CODEC_ERROR, FL_JSON_CODEC_ERROR_INVALID_NUMBER,
                "Can't encode NaN or Inf in JSON");
    return FALSE;
  }

  char text[G_ASCII_DTOSTR_BUF_SIZE];
  g_ascii_dtostr(text, G_ASCII_DTOSTR_BUF_SIZE, value);
  write_string(buffer, text);

  // Add .0 if no decimal point so not confused with an integer
  if (strchr(text, '.') == nullptr)
    write_string(buffer, ".0");

  return TRUE;
}

static char json_int_to_xdigit(int value) {
  return value < 10 ? '0' + value : 'a' + value;
}

static void write_unicode_escape(GByteArray* buffer, gunichar c) {
  write_string(buffer, "\\u");
  write_char(buffer, json_int_to_xdigit((c >> 24) & 0xF));
  write_char(buffer, json_int_to_xdigit((c >> 16) & 0xF));
  write_char(buffer, json_int_to_xdigit((c >> 8) & 0xF));
  write_char(buffer, json_int_to_xdigit((c >> 0) & 0xF));
}

static gboolean encode_value(GByteArray* buffer,
                             FlValue* value,
                             GError** error) {
  if (value == nullptr) {
    write_string(buffer, "null");
    return TRUE;
  }

  switch (fl_value_get_type(value)) {
    case FL_VALUE_TYPE_NULL:
      write_string(buffer, "null");
      break;
    case FL_VALUE_TYPE_BOOL:
      if (fl_value_get_bool(value))
        write_string(buffer, "true");
      else
        write_string(buffer, "false");
      break;
    case FL_VALUE_TYPE_INT:
      write_int(buffer, fl_value_get_int(value));
      break;
    case FL_VALUE_TYPE_FLOAT:
      if (!write_double(buffer, fl_value_get_float(value), error))
        return FALSE;
      break;
    case FL_VALUE_TYPE_STRING: {
      const gchar* string = fl_value_get_string(value);
      write_char(buffer, '\"');
      for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == '"')
          write_string(buffer, "\\\"");
        else if (string[i] == '\\')
          write_string(buffer, "\\\\");
        else if (string[i] == '\b')
          write_string(buffer, "\\b");
        else if (string[i] == '\f')
          write_string(buffer, "\\f");
        else if (string[i] == '\n')
          write_string(buffer, "\\n");
        else if (string[i] == '\r')
          write_string(buffer, "\\r");
        else if (string[i] == '\t')
          write_string(buffer, "\\t");
        else if (string[i] < 0x20)
          write_unicode_escape(buffer, string[i]);
        else
          write_char(buffer, string[i]);
      }
      write_char(buffer, '\"');
      break;
    }
    case FL_VALUE_TYPE_UINT8_LIST: {
      write_char(buffer, '[');
      const uint8_t* values = fl_value_get_uint8_list(value);
      for (size_t i = 0; i < fl_value_get_length(value); i++) {
        if (i != 0)
          write_char(buffer, ',');
        write_int(buffer, values[i]);
      }
      write_char(buffer, ']');
      break;
    }
    case FL_VALUE_TYPE_INT32_LIST: {
      write_char(buffer, '[');
      const int32_t* values = fl_value_get_int32_list(value);
      for (size_t i = 0; i < fl_value_get_length(value); i++) {
        if (i != 0)
          write_char(buffer, ',');
        write_int(buffer, values[i]);
      }
      write_char(buffer, ']');
      break;
    }
    case FL_VALUE_TYPE_INT64_LIST: {
      write_char(buffer, '[');
      const int64_t* values = fl_value_get_int64_list(value);
      for (size_t i = 0; i < fl_value_get_length(value); i++) {
        if (i != 0)
          write_char(buffer, ',');
        write_int(buffer, values[i]);
      }
      write_char(buffer, ']');
      break;
    }
    case FL_VALUE_TYPE_FLOAT_LIST: {
      write_char(buffer, '[');
      const double* values = fl_value_get_float_list(value);
      for (size_t i = 0; i < fl_value_get_length(value); i++) {
        if (i != 0)
          write_char(buffer, ',');
        if (!write_double(buffer, values[i], error))
          return FALSE;
      }
      write_char(buffer, ']');
      break;
    }
    case FL_VALUE_TYPE_LIST:
      write_char(buffer, '[');
      for (size_t i = 0; i < fl_value_get_length(value); i++) {
        if (i != 0)
          write_char(buffer, ',');
        if (!encode_value(buffer, fl_value_list_get_value(value, i), error))
          return FALSE;
      }
      write_char(buffer, ']');
      break;
    case FL_VALUE_TYPE_MAP:
      write_char(buffer, '{');
      for (size_t i = 0; i < fl_value_get_length(value); i++) {
        if (i != 0)
          write_char(buffer, ',');
        if (!encode_value(buffer, fl_value_map_get_key(value, i), error))
          return FALSE;
        write_char(buffer, ':');
        if (!encode_value(buffer, fl_value_map_get_value(value, i), error))
          return FALSE;
      }
      write_char(buffer, '}');
      break;
  }

  return TRUE;
}

static char current_char(const uint8_t* data,
                         size_t data_length,
                         size_t* offset) {
  if (*offset >= data_length)
    return '\0';
  else
    return data[*offset];
}

static void next_char(size_t* offset) {
  (*offset)++;
}

static gboolean is_json_whitespace(char value) {
  return value == ' ' || value == '\n' || value == '\r' || value == '\t';
}

static int json_digit_to_int(char value) {
  if (value >= '0' && value <= '9')
    return value - '0';
  else
    return -1;
}

static int json_xdigit_to_int(char value) {
  if (value >= '0' && value <= '9')
    return value - '0';
  else if (value >= 'a' && value <= 'f')
    return value - 'a' + 10;
  else if (value >= 'A' && value <= 'F')
    return value - 'A' + 10;
  else
    return -1;
}

static void skip_whitespace(const uint8_t* data,
                            size_t data_length,
                            size_t* offset) {
  while (*offset < data_length &&
         is_json_whitespace(current_char(data, data_length, offset)))
    next_char(offset);
}

static FlValue* decode_json_value(const uint8_t* data,
                                  size_t data_length,
                                  size_t* offset,
                                  GError** error);

static FlValue* decode_json_string(const uint8_t* data,
                                   size_t data_length,
                                   size_t* offset,
                                   GError** error);

static gboolean decode_comma(const uint8_t* data,
                             size_t data_length,
                             size_t* offset,
                             GError** error) {
  char c = current_char(data, data_length, offset);
  if (c != ',') {
    g_set_error(error, FL_JSON_CODEC_ERROR, FL_JSON_CODEC_ERROR_MISSING_COMMA,
                "Expected comma, got %02x", c);
    return FALSE;
  }
  next_char(offset);
  return TRUE;
}

static FlValue* decode_json_object(const uint8_t* data,
                                   size_t data_length,
                                   size_t* offset,
                                   GError** error) {
  g_assert(current_char(data, data_length, offset) == '{');
  next_char(offset);

  g_autoptr(FlValue) map = fl_value_map_new();
  while (TRUE) {
    skip_whitespace(data, data_length, offset);

    char c = current_char(data, data_length, offset);
    if (c == '\0') {
      g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA,
                  "Unterminated JSON object");
      return nullptr;
    }

    if (c == '}') {
      next_char(offset);
      return fl_value_ref(map);
    }

    if (fl_value_get_length(map) != 0) {
      if (!decode_comma(data, data_length, offset, error))
        return nullptr;
      skip_whitespace(data, data_length, offset);
    }

    c = current_char(data, data_length, offset);
    if (c != '\"') {
      g_set_error(error, FL_JSON_CODEC_ERROR,
                  FL_JSON_CODEC_ERROR_INVALID_OBJECT_KEY_TYPE,
                  "Missing string key in JSON object");
      return nullptr;
    }

    g_autoptr(FlValue) key =
        decode_json_string(data, data_length, offset, error);
    if (key == nullptr)
      return nullptr;
    skip_whitespace(data, data_length, offset);

    c = current_char(data, data_length, offset);
    if (c != ':') {
      g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                  "Missing colon after JSON object key");
      return nullptr;
    }
    next_char(offset);

    g_autoptr(FlValue) value =
        decode_json_value(data, data_length, offset, error);
    if (value == nullptr)
      return nullptr;

    fl_value_map_set(map, key, value);
  }
}

static FlValue* decode_json_array(const uint8_t* data,
                                  size_t data_length,
                                  size_t* offset,
                                  GError** error) {
  g_assert(current_char(data, data_length, offset) == '[');
  next_char(offset);

  g_autoptr(FlValue) value = fl_value_list_new();
  while (TRUE) {
    skip_whitespace(data, data_length, offset);

    char c = current_char(data, data_length, offset);
    if (c == '\0') {
      g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA,
                  "Unterminated JSON array");
      return nullptr;
    }

    if (c == ']') {
      next_char(offset);
      return fl_value_ref(value);
    }

    if (fl_value_get_length(value) != 0) {
      if (!decode_comma(data, data_length, offset, error))
        return nullptr;
      skip_whitespace(data, data_length, offset);
    }

    g_autoptr(FlValue) child =
        decode_json_value(data, data_length, offset, error);
    if (child == nullptr)
      return nullptr;

    fl_value_list_add(value, child);
  }
}

static gboolean decode_json_string_unichar(const uint8_t* data,
                                           size_t data_length,
                                           size_t* offset,
                                           gunichar* value,
                                           GError** error) {
  gunichar wc = 0;
  for (int i = 0; i < 4; i++) {
    char c = current_char(data, data_length, offset);
    int xdigit = json_xdigit_to_int(c);
    if (xdigit < 0) {
      g_set_error(error, FL_JSON_CODEC_ERROR,
                  FL_JSON_CODEC_ERROR_INVALID_STRING_UNICODE_ESCAPE,
                  "Missing hex digit in JSON unicode character");
      return FALSE;
    }
    wc = (wc << 4) + xdigit;
    next_char(offset);
  }

  *value = wc;
  return TRUE;
}

static gboolean decode_json_string_escape(const uint8_t* data,
                                          size_t data_length,
                                          size_t* offset,
                                          gunichar* value,
                                          GError** error) {
  char c = current_char(data, data_length, offset);
  if (c == 'u') {
    next_char(offset);
    return decode_json_string_unichar(data, data_length, offset, value, error);
  }

  if (c == '\"')
    *value = '\"';
  else if (c == '\\')
    *value = '\\';
  else if (c == '/')
    *value = '/';
  else if (c == 'b')
    *value = '\b';
  else if (c == 'f')
    *value = '\f';
  else if (c == 'n')
    *value = '\n';
  else if (c == 'r')
    *value = '\r';
  else if (c == 't')
    *value = '\t';
  else {
    g_set_error(error, FL_JSON_CODEC_ERROR,
                FL_JSON_CODEC_ERROR_INVALID_STRING_ESCAPE_SEQUENCE,
                "Unknown string escape character 0x%02x", c);
    return FALSE;
  }

  next_char(offset);
  return TRUE;
}

static FlValue* decode_json_string(const uint8_t* data,
                                   size_t data_length,
                                   size_t* offset,
                                   GError** error) {
  g_assert(current_char(data, data_length, offset) == '\"');
  next_char(offset);

  g_autoptr(GString) text = g_string_new("");
  while (TRUE) {
    char c = current_char(data, data_length, offset);
    if (c == '\"') {
      next_char(offset);
      return fl_value_string_new(text->str);
    } else if (c == '\\') {
      next_char(offset);
      gunichar wc = 0;
      if (!decode_json_string_escape(data, data_length, offset, &wc, error))
        return nullptr;
      g_string_append_unichar(text, wc);
      continue;
    } else if (c == '\0') {
      g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA,
                  "Unterminated string");
      return nullptr;
    } else if (c < 0x20) {
      g_set_error(error, FL_JSON_CODEC_ERROR,
                  FL_JSON_CODEC_ERROR_INVALID_STRING_CHARACTER,
                  "Invalid character in string");
      return nullptr;
    } else {
      g_string_append_c(text, c);
      next_char(offset);
    }
  }
}

static int64_t decode_json_digits(const uint8_t* data,
                                  size_t data_length,
                                  size_t* offset,
                                  int64_t* divisor) {
  int64_t value = 0;
  if (divisor != nullptr)
    *divisor = 1;

  while (TRUE) {
    char c = current_char(data, data_length, offset);
    if (json_digit_to_int(c) < 0)
      return value;
    value = value * 10 + json_digit_to_int(c);
    if (divisor != nullptr)
      (*divisor) *= 10;
    next_char(offset);
  }
}

static FlValue* decode_json_number(const uint8_t* data,
                                   size_t data_length,
                                   size_t* offset,
                                   GError** error) {
  char c = current_char(data, data_length, offset);
  int64_t sign = 1;
  if (c == '-') {
    sign = -1;
    next_char(offset);
    c = current_char(data, data_length, offset);
    if (json_digit_to_int(c) < 0) {
      g_set_error(error, FL_JSON_CODEC_ERROR,
                  FL_JSON_CODEC_ERROR_INVALID_NUMBER,
                  "Mising digits after negative sign");
      return nullptr;
    }
  }

  int64_t value = 0;
  if (c == '0')
    next_char(offset);
  else
    value = decode_json_digits(data, data_length, offset, nullptr);

  gboolean is_floating = FALSE;

  int64_t fraction = 0;
  int64_t divisor = 1;
  c = current_char(data, data_length, offset);
  if (c == '.') {
    is_floating = TRUE;
    next_char(offset);
    if (json_digit_to_int(current_char(data, data_length, offset)) < 0) {
      g_set_error(error, FL_JSON_CODEC_ERROR,
                  FL_JSON_CODEC_ERROR_INVALID_NUMBER,
                  "Mising digits after decimal point");
      return nullptr;
    }
    fraction = decode_json_digits(data, data_length, offset, &divisor);
  }

  int64_t exponent = 0;
  int64_t exponent_sign = 1;
  if (c == 'E' || c == 'e') {
    is_floating = TRUE;
    next_char(offset);

    c = current_char(data, data_length, offset);
    if (c == '-') {
      exponent_sign = -1;
      next_char(offset);
    } else if (c == '+') {
      exponent_sign = 1;
      next_char(offset);
    }

    if (json_digit_to_int(current_char(data, data_length, offset)) < 0) {
      g_set_error(error, FL_JSON_CODEC_ERROR,
                  FL_JSON_CODEC_ERROR_INVALID_NUMBER,
                  "Mising digits in exponent");
      return nullptr;
    }
    exponent = decode_json_digits(data, data_length, offset, nullptr);
  }

  if (is_floating)
    return fl_value_float_new(sign * (value + (double)fraction / divisor) *
                              pow(10, exponent_sign * exponent));
  else
    return fl_value_int_new(sign * value);
}

static gboolean decode_word(const uint8_t* data,
                            size_t data_length,
                            size_t* offset,
                            const gchar* word,
                            GError** error) {
  for (int i = 0; word[i] != '\0'; i++) {
    char c = current_char(data, data_length, offset);
    if (c != word[i]) {
      g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                  "Expected word %s not present", word);
      return FALSE;
    }
    next_char(offset);
  }

  return TRUE;
}

static FlValue* decode_json_true(const uint8_t* data,
                                 size_t data_length,
                                 size_t* offset,
                                 GError** error) {
  if (!decode_word(data, data_length, offset, "true", error))
    return nullptr;
  return fl_value_bool_new(TRUE);
}

static FlValue* decode_json_false(const uint8_t* data,
                                  size_t data_length,
                                  size_t* offset,
                                  GError** error) {
  if (!decode_word(data, data_length, offset, "false", error))
    return nullptr;
  return fl_value_bool_new(FALSE);
}

static FlValue* decode_json_null(const uint8_t* data,
                                 size_t data_length,
                                 size_t* offset,
                                 GError** error) {
  if (!decode_word(data, data_length, offset, "null", error))
    return nullptr;
  return fl_value_null_new();
}

static FlValue* decode_json_value(const uint8_t* data,
                                  size_t data_length,
                                  size_t* offset,
                                  GError** error) {
  skip_whitespace(data, data_length, offset);

  char c = current_char(data, data_length, offset);
  g_autoptr(FlValue) value = nullptr;
  if (c == '{')
    value = decode_json_object(data, data_length, offset, error);
  else if (c == '[')
    value = decode_json_array(data, data_length, offset, error);
  else if (c == '\"')
    value = decode_json_string(data, data_length, offset, error);
  else if (c == '-' || json_digit_to_int(c) >= 0)
    value = decode_json_number(data, data_length, offset, error);
  else if (c == 't')
    value = decode_json_true(data, data_length, offset, error);
  else if (c == 'f')
    value = decode_json_false(data, data_length, offset, error);
  else if (c == 'n')
    value = decode_json_null(data, data_length, offset, error);
  else if (c == '\0') {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA,
                "Out of data looking for JSON value");
    return nullptr;
  } else {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Unexpected value 0x%02x when decoding JSON value", c);
    return nullptr;
  }

  if (value == nullptr)
    return nullptr;

  skip_whitespace(data, data_length, offset);

  return fl_value_ref(value);
}

static gboolean fl_json_codec_write_value(FlCodec* codec,
                                          GByteArray* buffer,
                                          FlValue* value,
                                          GError** error) {
  return encode_value(buffer, value, error);
}

static FlValue* fl_json_codec_read_value(FlCodec* codec,
                                         GBytes* message,
                                         size_t* offset,
                                         GError** error) {
  gsize data_length;
  const uint8_t* data =
      static_cast<const uint8_t*>(g_bytes_get_data(message, &data_length));
  g_autoptr(FlValue) v = decode_json_value(data, data_length, offset, error);

  return v == nullptr ? nullptr : fl_value_ref(v);
}

static void fl_json_codec_class_init(FlJsonCodecClass* klass) {
  FL_CODEC_CLASS(klass)->write_value = fl_json_codec_write_value;
  FL_CODEC_CLASS(klass)->read_value = fl_json_codec_read_value;
}

static void fl_json_codec_init(FlJsonCodec* self) {}

G_MODULE_EXPORT FlJsonCodec* fl_json_codec_new() {
  return static_cast<FlJsonCodec*>(
      g_object_new(fl_json_codec_get_type(), nullptr));
}

G_MODULE_EXPORT gchar* fl_json_codec_encode(FlJsonCodec* codec,
                                            FlValue* value,
                                            GError** error) {
  g_autoptr(GByteArray) buffer = g_byte_array_new();
  if (!fl_codec_write_value(FL_CODEC(codec), buffer, value, error))
    return nullptr;
  guint8 nul = '\0';
  g_byte_array_append(buffer, &nul, 1);
  return reinterpret_cast<gchar*>(g_byte_array_free(
      static_cast<GByteArray*>(g_steal_pointer(&buffer)), FALSE));
}

G_MODULE_EXPORT FlValue* fl_json_codec_decode(FlJsonCodec* codec,
                                              const gchar* message,
                                              GError** error) {
  g_autoptr(GBytes) data = g_bytes_new_static(message, strlen(message));
  size_t offset = 0;
  g_autoptr(FlValue) value =
      fl_codec_read_value(FL_CODEC(codec), data, &offset, error);
  if (value == nullptr)
    return nullptr;
  if (offset != g_bytes_get_size(data)) {
    g_set_error(error, FL_JSON_CODEC_ERROR, FL_JSON_CODEC_ERROR_UNUSED_DATA,
                "Unexpected extra JSON data (%zi octets)",
                g_bytes_get_size(data) - offset);
    return nullptr;
  }
  return fl_value_ref(value);
}
