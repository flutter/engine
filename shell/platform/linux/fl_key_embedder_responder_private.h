// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EMBEDDER_RESPONDER_PRIVATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EMBEDDER_RESPONDER_PRIVATE_H_

#include <gdk/gdk.h>

#include "flutter/shell/platform/linux/fl_key_responder.h"
#include "flutter/shell/platform/linux/fl_keyboard_manager.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_value.h"

typedef struct {
  size_t length;
  uint64_t* physical_keys;
  uint64_t first_logical_key;
} FlKeyEmbedderCheckedKey;

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EMBEDDER_RESPONDER_PRIVATE_H_
