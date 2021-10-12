// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_text_editing_delta.h"

#include <gmodule.h>

#include <cstring>

G_MODULE_EXPORT FlTextEditingDelta* fl_text_editing_delta_new(
    std::string textBeforeChange,
    flutter::TextRange range,
    std::string text) {
  FlTextEditingDelta delta = FlTextEditingDelta();
  FlTextEditingDelta* self = &delta;
  self->textBeforeChange = textBeforeChange;
  self->range = range;
  self->text = text;
  return self;
}
