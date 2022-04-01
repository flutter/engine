// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_MANAGER_H_

#include <gdk/gdk.h>
#include <functional>

#include "flutter/shell/platform/linux/fl_keyboard_view_delegate.h"

/**
 * FlKeyboardManagerRedispatcher:
 * @event: the pointer to the event to dispatch.
 *
 * The signature for a callback with which a #FlKeyboardManager redispatches
 * key events that are not handled by anyone.
 *
 * The callee takes ownership of the received event, and is responsible for
 * disposing of it.
 **/
typedef std::function<void(std::unique_ptr<FlKeyEvent>)>
    FlKeyboardManagerRedispatcher;

G_BEGIN_DECLS

#define FL_TYPE_KEYBOARD_MANAGER fl_keyboard_manager_get_type()
G_DECLARE_FINAL_TYPE(FlKeyboardManager,
                     fl_keyboard_manager,
                     FL,
                     KEYBOARD_MANAGER,
                     GObject);

/**
 * FlKeyboardManager:
 *
 * A hub that manages how key events are dispatched to various processing
 * objects of Flutter, or possibly back to the system.
 *
 * This class manage one or more objects of #FlKeyResponder, as well as a
 * #TextInputPlugin.
 *
 * An event that is received by #fl_keyboard_manager_handle_event is first
 * dispatched to *all* responders. Each responder responds *ascynchronously*
 * with a boolean, indicating whether it handles the event.
 *
 * An event that is not handled by any responders is then passed to to the
 * #TextInputPlugin, which responds *synchronously* with a boolean, indicating
 * whether it handles the event.
 *
 * If no processing objects handle the event, the event is then "redispatched":
 * sent back to the system using #redispatch_callback.
 *
 * Preventing responders from receiving events is not supported, because in
 * reality this class will only support 2 hardcoded ones (channel and
 * embedder), where the only purpose of supporting two is to support the legacy
 * API (channel) during the deprecation window, after which the channel
 * responder should be removed.
 */

/**
 * fl_keyboard_manager_new:
 * @text_input_plugin: the #FlTextInputPlugin to send key events to if the
 * framework doesn't handle them. This object will be managed and freed by
 * #FlKeyboardManager.
 * @redispatch_callback: how the events should be sent if no processing
 * objects handle the event. Typically a function that calls #gdk_event_put
 * on #FlKeyEvent::origin.
 *
 * Create a new #FlKeyboardManager.
 *
 * Returns: a new #FlKeyboardManager.
 */
FlKeyboardManager* fl_keyboard_manager_new(
    FlKeyboardViewDelegate* view_delegate);

/**
 * fl_keyboard_manager_handle_event:
 * @manager: the #FlKeyboardManager self.
 * @event: the event to be dispatched. This event will be managed and
 * released by #FlKeyboardManager.
 *
 * Add a new #FlKeyResponder to the #FlKeyboardManager. Responders added
 * earlier will receive events earlier.
 */
gboolean fl_keyboard_manager_handle_event(FlKeyboardManager* manager,
                                          FlKeyEvent* event);

/**
 * fl_keyboard_manager_is_state_clear:
 * @manager: the #FlKeyboardManager self.
 *
 * Whether the manager's various states are cleared, i.e. no pending events
 * for redispatching or for responding. This is mostly used in unittests.
 *
 * Returns: true if the manager's various states are cleared.
 */
gboolean fl_keyboard_manager_is_state_clear(FlKeyboardManager* manager);

/**
 * fl_keyboard_manager_wait_for_pending:
 * @manager: the #FlKeyboardManager self.
 *
 * Block this thread until the all pending events are resolved.
 *
 * Asserts if a manager blocks while it's already blocking.
 *
 * This must only be used in unittests.
 */
void fl_keyboard_manager_wait_for_pending(FlKeyboardManager* manager);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_MANAGER_H_
