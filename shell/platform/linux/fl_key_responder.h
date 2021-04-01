// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_RESPONDER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_RESPONDER_H_

#include "flutter/shell/platform/linux/fl_text_input_plugin.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_value.h"

#include <gdk/gdk.h>
#include <cinttypes>

typedef void (*FlKeyResponderAsyncCallback)(FlKeyboardManager* manager, uint64_t sequence_id, bool handled);

G_BEGIN_DECLS

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
  bool (*handle_event)(FlKeyResponder* responder, GdkEvent* event, FlKeyResponderAsyncCallback callback, gpointer user_data);
};

/**
 * FlKeyResponder:
 *
 * #FlKeyResponder is a plugin that implements the shell side
 * of SystemChannels.keyEvent from the Flutter services library.
 */

bool fl_key_responder_handle_event(FlKeyResponder* responder, GdkEvent* event, FlKeyResponderAsyncCallback callback, gpointer user_data);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_RESPONDER_H_
