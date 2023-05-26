// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_PLUGIN_VIEW_DELEGATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_PLUGIN_VIEW_DELEGATE_H_

#include <gdk/gdk.h>
#include <cinttypes>
#include <memory>

G_BEGIN_DECLS

G_DECLARE_INTERFACE(FlKeyboardPluginViewDelegate,
                    fl_keyboard_plugin_view_delegate,
                    FL,
                    KEYBOARD_PLUGIN_VIEW_DELEGATE,
                    GObject);

/**
 * FlKeyboardPluginViewDelegate:
 *
 * An interface for a class that provides `FlKeyboardPlugin` with
 * view-related features.
 *
 * This interface is typically implemented by `FlView`.
 */

struct _FlKeyboardPluginViewDelegateInterface {
  GTypeInterface g_iface;

  GHashTable* (*get_keyboard_state)(FlKeyboardPluginViewDelegate* delegate);
};

GHashTable* fl_keyboard_plugin_view_delegate_get_keyboard_state(
    FlKeyboardPluginViewDelegate* self);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_PLUGIN_VIEW_DELEGATE_H_
