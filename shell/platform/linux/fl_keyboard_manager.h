// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_MANAGER_H_

#include <gdk/gdk.h>
#include <stdint.h>

#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

typedef enum {
  kFlKeyDataKindUp = 1,
  kFlKeyDataKindDown,
  kFlKeyDataKindRepeat,
} FlKeyEventKind;

typedef struct {
  FlKeyEventKind kind;
  double timestamp;
  uint64_t physical;
  uint64_t logical;
  uint32_t locks;
  const char* character;
  bool synthesized;
} FlKeyDatum;

constexpr int kMaxConvertedKeyData = 3;
constexpr int kMaxConvertedLogicalKeyData = 8;

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlKeyboardManager,
                     fl_keyboard_manager,
                     FL,
                     KEYBOARD_MANAGER,
                     GObject);

FlKeyboardManager* fl_keyboard_manager_new();

size_t fl_keyboard_manager_convert_key_event(FlKeyboardManager* self,
                                             const GdkEventKey* event,
                                             FlKeyDatum* result_physical,
                                             FlLogicalKeyDatum* result_logical);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_MANAGER_H_
