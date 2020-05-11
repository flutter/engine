// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_ACCESSIBILITY_CHANNEL_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_ACCESSIBILITY_CHANNEL_H_

#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlAccessibilityChannel,
                     fl_accessibility_channel,
                     FL,
                     ACCESSIBILITY_CHANNEL,
                     GObject);

/**
 * FlAccessibilityChannel:
 *
 * #FlAccessibilityChannel is a platform channel that implements the shell side
 * of SystemChannels.accessibility from the Flutter services library.
 */

/**
 * fl_accessibility_channel_new:
 * @messenger: an #FlBinaryMessenger
 *
 * Creates a new accessibility channel.
 *
 * Returns: a new #FlAccessibilityChannel
 */
FlAccessibilityChannel* fl_accessibility_channel_new(
    FlBinaryMessenger* messenger);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_ACCESSIBILITY_CHANNEL_H_
