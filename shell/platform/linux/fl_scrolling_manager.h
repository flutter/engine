// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_SCROLLING_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_SCROLLING_MANAGER_H_

#include <gdk/gdk.h>
#include <functional>

#include "flutter/shell/platform/linux/fl_scrolling_view_delegate.h"

G_BEGIN_DECLS

#define FL_TYPE_SCROLLING_MANAGER fl_scrolling_manager_get_type()
G_DECLARE_FINAL_TYPE(FlScrollingManager,
                     fl_scrolling_manager,
                     FL,
                     SCROLLING_MANAGER,
                     GObject);

/**
 * fl_keyboard_manager_new:
 * @view_delegate: An interface that the manager requires to communicate with
 * the platform. Usually implemented by FlView.
 *
 * Create a new #FlScrollingManager.
 *
 * Returns: a new #FlScrollingManager.
 */
FlScrollingManager* fl_scrolling_manager_new(
    FlScrollingViewDelegate* view_delegate);

void fl_scrolling_manager_set_last_mouse_position(FlScrollingManager* self,
                                                  gdouble x,
                                                  gdouble y);

void fl_scrolling_manager_handle_scroll_event(FlScrollingManager* self,
                                              GdkEventScroll* event,
                                              gint scale_factor);

void fl_scrolling_manager_handle_rotation_begin(FlScrollingManager* self);
void fl_scrolling_manager_handle_rotation_update(FlScrollingManager* self,
                                                 gdouble rotation);
void fl_scrolling_manager_handle_rotation_end(FlScrollingManager* self);

void fl_scrolling_manager_handle_zoom_begin(FlScrollingManager* self);
void fl_scrolling_manager_handle_zoom_update(FlScrollingManager* self,
                                             gdouble scale);
void fl_scrolling_manager_handle_zoom_end(FlScrollingManager* self);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_SCROLLING_MANAGER_H_
