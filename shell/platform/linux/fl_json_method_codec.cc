// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_json_method_codec.h"

#include "flutter/shell/platform/linux/public/flutter_linux/fl_json_codec.h"

#include <gmodule.h>

struct _FlJsonMethodCodec {
  FlMethodCodec parent_instance;

  FlJsonCodec* codec;
};

G_DEFINE_TYPE(FlJsonMethodCodec,
              fl_json_method_codec,
              fl_method_codec_get_type())

static void fl_json_method_codec_dispose(GObject* object) {
  FlJsonMethodCodec* self = FL_JSON_METHOD_CODEC(object);

  g_clear_object(&self->codec);

  G_OBJECT_CLASS(fl_json_method_codec_parent_class)->dispose(object);
}

static GBytes* fl_json_method_codec_encode_method_call(FlMethodCodec* codec,
                                                       const gchar* name,
                                                       FlValue* args,
                                                       GError** error) {
  FlJsonMethodCodec* self = FL_JSON_METHOD_CODEC(codec);

  g_autoptr(FlValue) request = fl_value_map_new();
  fl_value_map_set_take(request, fl_value_string_new("method"),
                        fl_value_string_new(name));
  fl_value_map_set_take(
      request, fl_value_string_new("args"),
      args != nullptr ? fl_value_ref(args) : fl_value_null_new());

  g_autoptr(GByteArray) buffer = g_byte_array_new();
  if (!fl_codec_write_value(FL_CODEC(self->codec), buffer, request, error))
    return nullptr;

  return g_byte_array_free_to_bytes(
      static_cast<GByteArray*>(g_steal_pointer(&buffer)));
}

static gboolean fl_json_method_codec_decode_method_call(FlMethodCodec* codec,
                                                        GBytes* message,
                                                        gchar** name,
                                                        FlValue** args,
                                                        GError** error) {
  FlJsonMethodCodec* self = FL_JSON_METHOD_CODEC(codec);

  size_t offset = 0;
  g_autoptr(FlValue) value =
      fl_codec_read_value(FL_CODEC(self->codec), message, &offset, error);
  if (value == nullptr)
    return FALSE;

  if (offset != g_bytes_get_size(message)) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Unexpected extra JSON data");
    return FALSE;
  }

  if (fl_value_get_type(value) != FL_VALUE_TYPE_MAP) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Expected JSON map in method resonse, got %d instead",
                fl_value_get_type(value));
    return FALSE;
  }

  FlValue* method_value = fl_value_map_lookup_string(value, "method");
  if (method_value == nullptr) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Missing JSON method field in method resonse");
    return FALSE;
  }
  if (fl_value_get_type(method_value) != FL_VALUE_TYPE_STRING) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Expected JSON string for method name, got %d instead",
                fl_value_get_type(method_value));
    return FALSE;
  }
  FlValue* args_value = fl_value_map_lookup_string(value, "args");

  *name = g_strdup(fl_value_get_string(method_value));
  *args = args_value != nullptr ? fl_value_ref(args_value) : nullptr;

  return TRUE;
}

static GBytes* fl_json_method_codec_encode_success_envelope(
    FlMethodCodec* codec,
    FlValue* result,
    GError** error) {
  FlJsonMethodCodec* self = FL_JSON_METHOD_CODEC(codec);

  g_autoptr(FlValue) value = fl_value_list_new();
  fl_value_list_add_take(
      value, result != nullptr ? fl_value_ref(result) : fl_value_null_new());

  g_autoptr(GByteArray) buffer = g_byte_array_new();
  if (!fl_codec_write_value(FL_CODEC(self->codec), buffer, value, error))
    return nullptr;

  return g_byte_array_free_to_bytes(
      static_cast<GByteArray*>(g_steal_pointer(&buffer)));
}

static GBytes* fl_json_method_codec_encode_error_envelope(FlMethodCodec* codec,
                                                          const gchar* code,
                                                          const gchar* message,
                                                          FlValue* details,
                                                          GError** error) {
  FlJsonMethodCodec* self = FL_JSON_METHOD_CODEC(codec);

  g_autoptr(FlValue) value = fl_value_list_new();
  fl_value_list_add_take(value, fl_value_string_new(code));
  fl_value_list_add_take(value, message != nullptr
                                    ? fl_value_string_new(message)
                                    : fl_value_null_new());
  fl_value_list_add_take(
      value, details != nullptr ? fl_value_ref(details) : fl_value_null_new());

  g_autoptr(GByteArray) buffer = g_byte_array_new();
  if (!fl_codec_write_value(FL_CODEC(self->codec), buffer, value, error))
    return nullptr;

  return g_byte_array_free_to_bytes(
      static_cast<GByteArray*>(g_steal_pointer(&buffer)));
}

static gboolean fl_json_method_codec_decode_response(FlMethodCodec* codec,
                                                     GBytes* message,
                                                     gchar** error_code,
                                                     gchar** error_message,
                                                     FlValue** result,
                                                     GError** error) {
  FlJsonMethodCodec* self = FL_JSON_METHOD_CODEC(codec);

  size_t offset = 0;
  g_autoptr(FlValue) value =
      fl_codec_read_value(FL_CODEC(self->codec), message, &offset, error);
  if (value == nullptr)
    return FALSE;

  if (offset != g_bytes_get_size(message)) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Unexpected extra JSON data");
    return FALSE;
  }

  if (fl_value_get_type(value) != FL_VALUE_TYPE_LIST) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Expected JSON list in method resonse, got %d instead",
                fl_value_get_type(value));
    return FALSE;
  }

  size_t length = fl_value_get_length(value);
  if (length == 1) {
    *error_code = nullptr;
    *error_message = nullptr;
    *result = fl_value_ref(fl_value_list_get_value(value, 0));
  } else if (length == 3) {
    FlValue* code_value = fl_value_list_get_value(value, 0);
    if (fl_value_get_type(code_value) != FL_VALUE_TYPE_STRING) {
      g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                  "Error code wrong type");
      return FALSE;
    }

    FlValue* message_value = fl_value_list_get_value(value, 1);
    if (fl_value_get_type(message_value) != FL_VALUE_TYPE_STRING &&
        fl_value_get_type(message_value) != FL_VALUE_TYPE_NULL) {
      g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                  "Error message wrong type");
      return FALSE;
    }

    *error_code = g_strdup(fl_value_get_string(code_value));
    *error_message = fl_value_get_type(message_value) == FL_VALUE_TYPE_STRING
                         ? g_strdup(fl_value_get_string(message_value))
                         : nullptr;
    *result = fl_value_ref(fl_value_list_get_value(value, 2));
  } else {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_FAILED,
                "Got response envelope of length %zi, expected 1 (success) or "
                "3 (error)",
                length);
    return FALSE;
  }

  return TRUE;
}

static void fl_json_method_codec_class_init(FlJsonMethodCodecClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_json_method_codec_dispose;
  FL_METHOD_CODEC_CLASS(klass)->encode_method_call =
      fl_json_method_codec_encode_method_call;
  FL_METHOD_CODEC_CLASS(klass)->decode_method_call =
      fl_json_method_codec_decode_method_call;
  FL_METHOD_CODEC_CLASS(klass)->encode_success_envelope =
      fl_json_method_codec_encode_success_envelope;
  FL_METHOD_CODEC_CLASS(klass)->encode_error_envelope =
      fl_json_method_codec_encode_error_envelope;
  FL_METHOD_CODEC_CLASS(klass)->decode_response =
      fl_json_method_codec_decode_response;
}

static void fl_json_method_codec_init(FlJsonMethodCodec* self) {
  self->codec = fl_json_codec_new();
}

G_MODULE_EXPORT FlJsonMethodCodec* fl_json_method_codec_new() {
  return static_cast<FlJsonMethodCodec*>(
      g_object_new(fl_json_method_codec_get_type(), nullptr));
}
