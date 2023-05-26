// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_PLUGIN_H_

#include <gdk/gdk.h>

#include "flutter/shell/platform/linux/fl_keyboard_plugin_view_delegate.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

G_BEGIN_DECLS

#define FL_TYPE_KEYBOARD_PLUGIN fl_keyboard_plugin_get_type()
G_DECLARE_FINAL_TYPE(FlKeyboardPlugin,
                     fl_keyboard_plugin,
                     FL,
                     KEYBOARD_PLUGIN,
                     GObject);

/**
 * FlKeyboardPlugin:
 *
 * #FlKeyboardPlugin is a keyboard channel that implements the shell side
 * of SystemChannels.keyboard from the Flutter services library.
 */

/**
 * fl_keyboard_plugin_new:
 * @messenger: an #FlBinaryMessenger.
 * @view_delegate: An interface that the plugin requires to communicate with
 * the platform. Usually implemented by FlView.
 *
 * Creates a new plugin that implements SystemChannels.keyboard from the
 * Flutter services library.
 *
 * Returns: a new #FlKeyboardPlugin.
 */
FlKeyboardPlugin* fl_keyboard_plugin_new(
    FlBinaryMessenger* messenger,
    FlKeyboardPluginViewDelegate* view_delegate);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_PLUGIN_H_
