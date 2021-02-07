// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_CLIPPING_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_CLIPPING_VIEW_H_

#include <gtk/gtk.h>

#include <glib-object.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlClippingView,
                     fl_clipping_view,
                     FL,
                     CLIPPING_VIEW,
                     GtkEventBox)

/**
 * FlClippingView:
 *
 * #FlClippingView is a GTK widget that draws its child with mutators.
 */

/**
 * fl_clipping_view_new:
 *
 * Creates a new #FlClippingView widget.
 *
 * Returns: the newly created #FlClippingView widget.
 */
GtkWidget* fl_clipping_view_new();

/**
 * fl_clipping_view_reset:
 * @clipping_view: an #FlClippingView.
 * @child: (transfer-none): widget to apply mutators.
 * @geometry: geometry of widget.
 * @mutations: (transfer-full): mutations to be applied.
 *
 * Reset child widget and its mutations.
 */
void fl_clipping_view_reset(FlClippingView* clipping_view,
                            GtkWidget* child,
                            GdkRectangle* geometry,
                            GPtrArray* mutations);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_CLIPPING_VIEW_H_
