// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_MANAGER_H_

#include <gdk/gdk.h>

#include "flutter/shell/platform/linux/fl_text_input_plugin.h"
#include "flutter/shell/platform/linux/fl_key_responder.h"

/**
 * FlKeyEventPluginCallback:
 * @source_object: (nullable): the object the key event was started with.
 * @message: the message returned from the framework.
 * @handled: a boolean indicating whether the key event was handled in the
 *           framework.
 * @user_data: user data passed to the callback.
 *
 * Type definition for a function that will be called when a key event is
 * received from the engine.
 **/
typedef void (*FlKeyboardManagerRedispatcher)(const GdkEvent* event);

G_BEGIN_DECLS

#define FL_KEYBOARD_MANAGER fl_keyboard_manager_get_type
G_DECLARE_FINAL_TYPE(FlKeyboardManager,
                     fl_keyboard_manager,
                     FL,
                     KEYBOARD_MANAGER,
                     GObject);

/**
 * FlKeyboardManager:
 *
 * #FlKeyboardManager is a plugin that implements the shell side
 * of SystemChannels.keyEvent from the Flutter services library.
 */

/**
 * fl_keyboard_manager_new:
 * @messenger: an #FlBinaryMessenger.
 * @response_callback: the callback to call when a response is received.  If not
 *                     given (nullptr), then the default response callback is
 *                     used. Typically used for tests to receive event
 *                     information. If specified, unhandled events will not be
 *                     re-dispatched.
 * @text_input_plugin: The #FlTextInputPlugin to send key events to if the
 *                     framework doesn't handle them.
 * @channel_name: the name of the channel to send key events to the framework
 *                on. If not given (nullptr), then the standard key event
 *                channel name is used. Typically used for tests to send on a
 *                test channel.
 *
 * Creates a new plugin that implements SystemChannels.keyEvent from the
 * Flutter services library.
 *
 * Returns: a new #FlKeyEventPlugin.
 */
FlKeyboardManager* fl_keyboard_manager_new(
    FlTextInputPlugin* text_input_plugin,
    FlKeyboardManagerRedispatcher redispatch_callback);

void fl_keyboard_manager_add_responder(
    FlKeyboardManager* manager,
    FlKeyResponder* responder);

gboolean fl_keyboard_manager_handle_event(FlKeyboardManager* manager, GdkEventKey* event);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_MANAGER_H_
