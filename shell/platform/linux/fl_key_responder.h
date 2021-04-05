// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_RESPONDER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_RESPONDER_H_

#include <gdk/gdk.h>
#include <cinttypes>

#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_value.h"

G_BEGIN_DECLS

typedef struct _FlKeyboardManager FlKeyboardManager;
typedef void (*FlKeyResponderAsyncCallback)(bool handled, gpointer user_data);

#define FL_TYPE_KEY_RESPONDER fl_key_responder_get_type ()

G_DECLARE_INTERFACE(FlKeyResponder,
                    fl_key_responder,
                    FL,
                    KEY_RESPONDER,
                    GObject);

struct _FlKeyResponderInterface {
  GTypeInterface g_iface;

  /**
   * FlPluginRegistry::get_registrar_for_plugin:
   * @registry: an #FlPluginRegistry.
   * @name: plugin name.
   *
   * Gets the plugin registrar for the the plugin with @name.
   *
   * Returns: (transfer full): an #FlPluginRegistrar.
   */
  void (*handle_event)(FlKeyResponder* responder, GdkEventKey* event, FlKeyResponderAsyncCallback callback, gpointer user_data);
};

/**
 * FlKeyResponder:
 *
 * #FlKeyResponder is a plugin that implements the shell side
 * of SystemChannels.keyEvent from the Flutter services library.
 */

void fl_key_responder_handle_event(FlKeyResponder* responder, GdkEventKey* event, FlKeyResponderAsyncCallback callback, gpointer user_data);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_RESPONDER_H_
