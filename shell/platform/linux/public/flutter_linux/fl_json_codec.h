// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_JSON_CODEC_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_JSON_CODEC_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include "fl_codec.h"

G_BEGIN_DECLS

/**
 * FlJsonCodecError:
 * Errors for #FlJsonCodec objects to set on failures.
 */
#define FL_JSON_CODEC_ERROR fl_json_codec_error_quark()

typedef enum {
  FL_JSON_CODEC_ERROR_MISSING_COMMA,
  FL_JSON_CODEC_ERROR_INVALID_NUMBER,
  FL_JSON_CODEC_ERROR_INVALID_STRING_CHARACTER,
  FL_JSON_CODEC_ERROR_INVALID_STRING_ESCAPE_SEQUENCE,
  FL_JSON_CODEC_ERROR_INVALID_STRING_UNICODE_ESCAPE,
  FL_JSON_CODEC_ERROR_INVALID_OBJECT_KEY_TYPE,
  FL_JSON_CODEC_ERROR_UNUSED_DATA,
} FlJsonCodecError;

GQuark fl_json_codec_error_quark(void) G_GNUC_CONST;

G_DECLARE_FINAL_TYPE(FlJsonCodec, fl_json_codec, FL, JSON_CODEC, FlCodec)

/**
 * FlJsonCodec:
 *
 * #FlJsonCodec is a #FlCodec that implements the Flutter JSON message encoding.
 */

FlJsonCodec* fl_json_codec_new();

/**
 * fl_codec_encode:
 * @codec: a #FlJsonCodec
 * @value: value to encode
 * @error: (allow-none): #GError location to store the error occurring, or %NULL
 *
 * Encode a value to a JSON string.
 *
 * Returns: a JSON representation of this value or %NULL on error.
 */
gchar* fl_json_codec_encode(FlJsonCodec* codec, FlValue* value, GError** error);

/**
 * fl_codec_decode:
 * @codec: a #FlJsonCodec
 * @message: message to decode
 * @error: (allow-none): #GError location to store the error occurring, or %NULL
 *
 * Decode a value from a JSON string.
 *
 * Returns: a #FlValue or %NULL on error
 */
FlValue* fl_json_codec_decode(FlJsonCodec* codec,
                              const gchar* message,
                              GError** error);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_JSON_CODEC_H_
