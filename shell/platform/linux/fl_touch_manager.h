// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_TOUCH_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_TOUCH_MANAGER_H_

#include <gdk/gdk.h>

#include "flutter/shell/platform/linux/fl_touch_view_delegate.h"
#include "flutter/shell/platform/linux/sequential_id_generator.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlTouchManager,
                     fl_touch_manager,
                     FL,
                     TOUCH_MANAGER,
                     GObject);

/**
 * fl_touch_manager_new:
 * @view_delegate: An interface that the manager requires to communicate with
 * the platform. Usually implemented by FlView.
 *
 * Create a new #FlTouchManager.
 *
 * Returns: a new #FlTouchManager.
 */
FlTouchManager* fl_touch_manager_new(
    FlTouchViewDelegate* view_delegate);

/**
 * fl_touch_manager_handle_touch_event:
 * @manager: an #FlTouchManager.
 * @event: the touch event.
 * @scale_factor: the GTK scaling factor of the window.
 */
void fl_touch_manager_handle_touch_event(FlTouchManager* manager,
                                         GdkEventTouch* event,
                                         gint scale_factor);


G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_TOUCH_MANAGER_H_
