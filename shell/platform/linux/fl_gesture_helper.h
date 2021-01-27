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

FlGestureHelper* fl_gesture_helper_new();

void fl_gesture_helper_button_press(FlGestureHelper* gesture_helper,
                                    GdkEvent* event);

void fl_gesture_helper_button_release(FlGestureHelper* gesture_helper,
                                      GdkEvent* event);

void fl_gesture_helper_button_motion(FlGestureHelper* gesture_helper,
                                     GdkEvent* event);

void fl_gesture_helper_accept_gesture(FlGestureHelper* gesture_helper,
                                      GtkWidget* widget,
                                      int64_t pointer_id);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_GESTURE_HELPER_H_
