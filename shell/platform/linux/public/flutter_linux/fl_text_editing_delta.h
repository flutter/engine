// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXT_EDITING_DELTA_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXT_EDITING_DELTA_H_

#include <glib.h>
#include <stdbool.h>
#include <stdint.h>

#include "flutter/shell/platform/common/text_input_model.h"

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

G_BEGIN_DECLS

struct FlTextEditingDelta {
  FlTextEditingDelta() = default;

  std::string textBeforeChange;

  flutter::TextRange range = flutter::TextRange(0, 0);

  std::string text;
};

/**
 * fl_text_editing_delta_new:
 * @textBeforeChange: the text before applying the delta.
 * @range: the range in textBeforeChange that the delta affects.
 * @text: the text that replaces the range in textBeforeChange.
 *
 * Creates a new FlTextEditingDelta.
 *
 * Returns: a new #FlTextEditingDelta.
 */
FlTextEditingDelta* fl_text_editing_delta_new(std::string textBeforeChange,
                                              flutter::TextRange range,
                                              std::string text);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXT_EDITING_DELTA_H_
