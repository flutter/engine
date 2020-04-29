// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_STANDARD_CODEC_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_STANDARD_CODEC_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include "fl_codec.h"

G_BEGIN_DECLS

/**
 * FlStandardCodecError:
 * Errors for #FlStandardCodec objects to set on failures.
 */
#define FL_STANDARD_CODEC_ERROR fl_standard_codec_error_quark()

typedef enum {
  FL_STANDARD_CODEC_ERROR_UNKNOWN_TYPE,
} FlStandardCodecError;

GQuark fl_standard_codec_error_quark(void) G_GNUC_CONST;

G_DECLARE_FINAL_TYPE(FlStandardCodec,
                     fl_standard_codec,
                     FL,
                     STANDARD_CODEC,
                     FlCodec)

/**
 * FlStandardCodec:
 *
 * #FlStandardCodec is a #FlCodec that implements the Flutter standard message
 * encoding.
 */

FlStandardCodec* fl_standard_codec_new();

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_STANDARD_CODEC_H_
