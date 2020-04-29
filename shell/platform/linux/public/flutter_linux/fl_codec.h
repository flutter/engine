// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_CODEC_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_CODEC_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <glib-object.h>

#include "fl_value.h"

G_BEGIN_DECLS

/**
 * FlCodecError:
 * Errors for #FlCodec objects to set on failures.
 */
#define FL_CODEC_ERROR fl_codec_error_quark()

typedef enum {
  FL_CODEC_ERROR_FAILED,
  FL_CODEC_ERROR_OUT_OF_DATA,
} FlCodecError;

GQuark fl_codec_error_quark(void) G_GNUC_CONST;

G_DECLARE_DERIVABLE_TYPE(FlCodec, fl_codec, FL, CODEC, GObject)

/**
 * FlCodec:
 *
 * #FlCodec is an abstract class that converts #FlValue to and from a binary
 * representation.
 */

struct _FlCodecClass {
  GObjectClass parent_class;

  /**
   * FlCodec::write_value:
   * @codec: A #FlCodec
   * @buffer: buffer to write to
   * @value: value to encode or %NULL to encode the null value.
   * @error: (allow-none): #GError location to store the error occurring, or
   * %NULL
   *
   * Encode a value into the buffer.
   *
   * Returns: %TRUE on success.
   */
  gboolean (*write_value)(FlCodec* codec,
                          GByteArray* buffer,
                          FlValue* value,
                          GError** error);

  /**
   * FlCodec::read_value:
   * @codec: a #FlCodec
   * @buffer: buffer to read from
   * @offset: (allow-none): index to read from buffer or %NULL to read from the
   * start. If provided, this value will be increased by the amount of data this
   * value used in the buffer.
   * @error: (allow-none): #GError location to store the error occurring, or
   * %NULL
   *
   * Decode a value from a binary encoding.
   *
   * Returns: a #FlValue or %NULL on error.
   */
  FlValue* (*read_value)(FlCodec* codec,
                         GBytes* buffer,
                         size_t* offset,
                         GError** error);
};

/**
 * fl_codec_write_value:
 * @codec: a #FlCodec
 * @buffer: buffer to write to
 * @value: value to encode or %NULL to encode the null value.
 * @error: (allow-none): #GError location to store the error occurring, or %NULL
 *
 * Encode a value into the buffer.
 *
 * Returns: %TRUE on success.
 */
gboolean fl_codec_write_value(FlCodec* codec,
                              GByteArray* buffer,
                              FlValue* value,
                              GError** error);

/**
 * fl_codec_read_value:
 * @codec: a #FlCodec
 * @buffer: buffer to read from
 * @offset: (allow-none): index to read from buffer or %NULL to read from the
 * start. If provided, this value will be increased by the amount of data this
 * value used in the buffer.
 * @error: (allow-none): #GError location to store the error occurring, or %NULL
 *
 * Decode a value from a binary encoding.
 *
 * Returns: a #FlValue or %NULL on error.
 */
FlValue* fl_codec_read_value(FlCodec* codec,
                             GBytes* buffer,
                             size_t* offset,
                             GError** error);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_CODEC_H_
