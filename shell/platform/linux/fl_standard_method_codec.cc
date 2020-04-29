// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_method_codec.h"

#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_codec.h"

#include <gmodule.h>

static constexpr guint8 kEnvelopeTypeSuccess = 0;
static constexpr guint8 kEnvelopeTypeError = 1;

struct _FlStandardMethodCodec {
  FlMethodCodec parent_instance;

  FlStandardCodec* codec;
};

G_DEFINE_TYPE(FlStandardMethodCodec,
              fl_standard_method_codec,
              fl_method_codec_get_type())

static void fl_standard_method_codec_dispose(GObject* object) {
  FlStandardMethodCodec* self = FL_STANDARD_METHOD_CODEC(object);

  g_clear_object(&self->codec);

  G_OBJECT_CLASS(fl_standard_method_codec_parent_class)->dispose(object);
}

static GBytes* fl_standard_method_codec_encode_method_call(FlMethodCodec* codec,
                                                           const gchar* name,
                                                           FlValue* args,
                                                           GError** error) {
  FlStandardMethodCodec* self = FL_STANDARD_METHOD_CODEC(codec);

  g_autoptr(GByteArray) buffer = g_byte_array_new();
  g_autoptr(FlValue) name_value = fl_value_string_new(name);
  if (!fl_codec_write_value(FL_CODEC(self->codec), buffer, name_value, error))
    return nullptr;
  if (!fl_codec_write_value(FL_CODEC(self->codec), buffer, args, error))
    return nullptr;

  return g_byte_array_free_to_bytes(
      static_cast<GByteArray*>(g_steal_pointer(&buffer)));
}

static gboolean fl_standard_method_codec_decode_method_call(
    FlMethodCodec* codec,
    GBytes* message,
    gchar** name,
    FlValue** args,
    GError** error) {
  FlStandardMethodCodec* self = FL_STANDARD_METHOD_CODEC(codec);

  size_t offset = 0;
  g_autoptr(FlValue) name_value =
      fl_codec_read_value(FL_CODEC(self->codec), message, &offset, error);
  if (name_value == nullptr)
    return FALSE;
  if (fl_value_get_type(name_value) != FL_VALUE_TYPE_STRING) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Method call name wrong type");
    return FALSE;
  }

  g_autoptr(FlValue) args_value =
      fl_codec_read_value(FL_CODEC(self->codec), message, &offset, error);
  if (args_value == nullptr)
    return FALSE;

  if (offset != g_bytes_get_size(message)) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Unexpected extra data");
    return FALSE;
  }

  *name = g_strdup(fl_value_get_string(name_value));
  *args = fl_value_ref(args_value);

  return TRUE;
}

static GBytes* fl_standard_method_codec_encode_success_envelope(
    FlMethodCodec* codec,
    FlValue* result,
    GError** error) {
  FlStandardMethodCodec* self = FL_STANDARD_METHOD_CODEC(codec);

  g_autoptr(GByteArray) buffer = g_byte_array_new();
  guint8 type = kEnvelopeTypeSuccess;
  g_byte_array_append(buffer, &type, 1);
  if (!fl_codec_write_value(FL_CODEC(self->codec), buffer, result, error))
    return nullptr;

  return g_byte_array_free_to_bytes(
      static_cast<GByteArray*>(g_steal_pointer(&buffer)));
}

static GBytes* fl_standard_method_codec_encode_error_envelope(
    FlMethodCodec* codec,
    const gchar* code,
    const gchar* message,
    FlValue* details,
    GError** error) {
  FlStandardMethodCodec* self = FL_STANDARD_METHOD_CODEC(codec);

  g_autoptr(GByteArray) buffer = g_byte_array_new();
  guint8 type = kEnvelopeTypeError;
  g_byte_array_append(buffer, &type, 1);
  g_autoptr(FlValue) code_value = fl_value_string_new(code);
  if (!fl_codec_write_value(FL_CODEC(self->codec), buffer, code_value, error))
    return nullptr;
  g_autoptr(FlValue) message_value =
      message != nullptr ? fl_value_string_new(message) : nullptr;
  if (!fl_codec_write_value(FL_CODEC(self->codec), buffer, message_value,
                            error))
    return nullptr;
  if (!fl_codec_write_value(FL_CODEC(self->codec), buffer, details, error))
    return nullptr;

  return g_byte_array_free_to_bytes(
      static_cast<GByteArray*>(g_steal_pointer(&buffer)));
}

static gboolean fl_standard_method_codec_decode_response(FlMethodCodec* codec,
                                                         GBytes* message,
                                                         gchar** error_code,
                                                         gchar** error_message,
                                                         FlValue** result,
                                                         GError** error) {
  FlStandardMethodCodec* self = FL_STANDARD_METHOD_CODEC(codec);

  if (g_bytes_get_size(message) == 0) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA,
                "Empty response");
    return FALSE;
  }

  const guint8* data =
      static_cast<const guint8*>(g_bytes_get_data(message, nullptr));
  guint8 type = data[0];
  size_t offset = 1;

  g_autoptr(FlValue) code_value = nullptr;
  g_autoptr(FlValue) message_value = nullptr;
  if (type == kEnvelopeTypeError) {
    code_value =
        fl_codec_read_value(FL_CODEC(self->codec), message, &offset, error);
    if (code_value == nullptr)
      return FALSE;
    if (fl_value_get_type(code_value) != FL_VALUE_TYPE_STRING) {
      g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                  "Error code wrong type");
      return FALSE;
    }

    message_value =
        fl_codec_read_value(FL_CODEC(self->codec), message, &offset, error);
    if (message_value == nullptr)
      return FALSE;
    if (fl_value_get_type(message_value) != FL_VALUE_TYPE_STRING &&
        fl_value_get_type(message_value) != FL_VALUE_TYPE_NULL) {
      g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                  "Error message wrong type");
      return FALSE;
    }
  } else if (type != kEnvelopeTypeSuccess) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Unknown envelope type %02x", type);
    return FALSE;
  }

  g_autoptr(FlValue) result_value =
      fl_codec_read_value(FL_CODEC(self->codec), message, &offset, error);
  if (result_value == nullptr)
    return FALSE;

  if (offset != g_bytes_get_size(message)) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Unexpected extra data");
    return FALSE;
  }

  *error_code = code_value != nullptr
                    ? g_strdup(fl_value_get_string(code_value))
                    : nullptr;
  *error_message =
      message_value != nullptr &&
              fl_value_get_type(message_value) == FL_VALUE_TYPE_STRING
          ? g_strdup(fl_value_get_string(message_value))
          : nullptr;
  *result = fl_value_ref(result_value);

  return TRUE;
}

static void fl_standard_method_codec_class_init(
    FlStandardMethodCodecClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_standard_method_codec_dispose;
  FL_METHOD_CODEC_CLASS(klass)->encode_method_call =
      fl_standard_method_codec_encode_method_call;
  FL_METHOD_CODEC_CLASS(klass)->decode_method_call =
      fl_standard_method_codec_decode_method_call;
  FL_METHOD_CODEC_CLASS(klass)->encode_success_envelope =
      fl_standard_method_codec_encode_success_envelope;
  FL_METHOD_CODEC_CLASS(klass)->encode_error_envelope =
      fl_standard_method_codec_encode_error_envelope;
  FL_METHOD_CODEC_CLASS(klass)->decode_response =
      fl_standard_method_codec_decode_response;
}

static void fl_standard_method_codec_init(FlStandardMethodCodec* self) {
  self->codec = fl_standard_codec_new();
}

G_MODULE_EXPORT FlStandardMethodCodec* fl_standard_method_codec_new() {
  FlStandardMethodCodec* self = static_cast<FlStandardMethodCodec*>(
      g_object_new(fl_standard_method_codec_get_type(), nullptr));
  return self;
}
