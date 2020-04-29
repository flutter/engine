// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_codec.h"

#include <gmodule.h>

G_DEFINE_QUARK(fl_codec_error_quark, fl_codec_error)

// Added here to stop the compiler from optimising this function away
G_MODULE_EXPORT GType fl_codec_get_type();

G_DEFINE_TYPE(FlCodec, fl_codec, G_TYPE_OBJECT)

static void fl_codec_class_init(FlCodecClass* klass) {}

static void fl_codec_init(FlCodec* self) {}

G_MODULE_EXPORT gboolean fl_codec_write_value(FlCodec* self,
                                              GByteArray* buffer,
                                              FlValue* value,
                                              GError** error) {
  g_return_val_if_fail(FL_IS_CODEC(self), FALSE);
  g_return_val_if_fail(buffer != nullptr, FALSE);

  g_autoptr(FlValue) null_value = NULL;
  if (value == nullptr)
    value = null_value = fl_value_null_new();

  return FL_CODEC_GET_CLASS(self)->write_value(self, buffer, value, error);
}

G_MODULE_EXPORT FlValue* fl_codec_read_value(FlCodec* self,
                                             GBytes* buffer,
                                             size_t* offset,
                                             GError** error) {
  g_return_val_if_fail(FL_IS_CODEC(self), nullptr);
  g_return_val_if_fail(buffer != nullptr, nullptr);

  size_t o = offset != nullptr ? *offset : 0;
  if (o >= g_bytes_get_size(buffer)) {
    g_set_error(error, FL_CODEC_ERROR, FL_CODEC_ERROR_OUT_OF_DATA,
                "Out of data");
    return NULL;
  }

  FlValue* value =
      FL_CODEC_GET_CLASS(self)->read_value(self, buffer, &o, error);
  if (value != nullptr && offset != nullptr)
    *offset = o;
  return value;
}
