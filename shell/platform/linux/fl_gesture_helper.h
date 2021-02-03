// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_GESTURE_HELPER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_GESTURE_HELPER_H_

#include <gtk/gtk.h>

#include <glib-object.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlGestureHelper,
                     fl_gesture_helper,
                     FL,
                     GESTURE_HELPER,
                     GObject)

/**
 * FlGestureHelper:
 *
 * #FlGestureHelper captures and redistributes mouse events to embedded platform
 * views.
 */

/**
 * fl_gesture_helper_new:
 *
 * Creates a new #FlGestureHelper object.
 *
 * Returns: a new #FlGestureHelper.
 */
FlGestureHelper* fl_gesture_helper_new();

/**
 * fl_gesture_helper_button_press:
 * @gesture_helper: an #FlGestureHelper.
 * @event: a #GdkEventButton event.
 *
 * Redistrubutes @event to a embedded platform view.
 */
void fl_gesture_helper_button_press(FlGestureHelper* gesture_helper,
                                    GdkEvent* event);

/**
 * fl_gesture_helper_button_release:
 * @gesture_helper: an #FlGestureHelper.
 * @event: a #GdkEventButton event.
 *
 * Redistributes @event to a embeeded platform view.
 */
void fl_gesture_helper_button_release(FlGestureHelper* gesture_helper,
                                      GdkEvent* event);

/**
 * fl_gesture_helper_button_motion:
 * @gesture_helper: an #FlGestureHelper.
 * @event: a #GdkEventMotion event.
 *
 * Redistributes a #GdkEventMotion event to embedded platform views.
 */
void fl_gesture_helper_button_motion(FlGestureHelper* gesture_helper,
                                     GdkEvent* event);

/**
 * fl_gesture_helper_accept_gesture:
 * @gesture_helper: an #FlGestureHelper.
 * @widget: the widget to distribute current gesture to.
 * @pointer_id: touch sequence id.
 *
 * Distributes all delayed and following pointer events of current gesture to
 * @widget.
 */
void fl_gesture_helper_accept_gesture(FlGestureHelper* gesture_helper,
                                      GtkWidget* widget,
                                      int64_t pointer_id);

/**
 * fl_gesture_helper_enter:
 * @gesture_helper: an #FlGestureHelper.
 * @widget: the widget that a mouse pointer enters into.
 *
 * Notify that a mouse pointer enters into @widget, and distribute following
 * motion events to @widget.
 */
void fl_gesture_helper_enter(FlGestureHelper* gesture_helper,
                             GtkWidget* widget);

/**
 * fl_gesture_helper_exit:
 * @gesture_helper: an #FlGestureHelper.
 * @widget: the widget that a mouse pointer exits from.
 *
 * Notify that a mouse pointer exits from @widget, and stop distributing motion
 * events to @widget.
 */
void fl_gesture_helper_exit(FlGestureHelper* gesture_helper, GtkWidget* widget);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_GESTURE_HELPER_H_
