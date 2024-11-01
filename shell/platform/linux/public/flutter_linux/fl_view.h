// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_PUBLIC_FLUTTER_LINUX_FL_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_PUBLIC_FLUTTER_LINUX_FL_VIEW_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <gmodule.h>
#include <gtk/gtk.h>

#include "fl_dart_project.h"
#include "fl_engine.h"

G_BEGIN_DECLS

G_MODULE_EXPORT
G_DECLARE_INTERFACE(FlView, fl_view, FL, VIEW, GObject)

/**
 * FlView:
 *
 * #FlView is an interface representing a Flutter view.
 *
 * The following example shows how to set up a view in a GTK application:
 * |[<!-- language="C" -->
 *   FlDartProject *project = fl_dart_project_new ();
 *   FlView *view = fl_view_widget_new (project);
 *   gtk_widget_show (GTK_WIDGET (view));
 *   gtk_container_add (GTK_CONTAINER (parent), view);
 *
 *   FlBinaryMessenger *messenger =
 *     fl_engine_get_binary_messenger (fl_view_get_engine (view));
 *   setup_channels_or_plugins (messenger);
 * ]|
 */

struct _FlViewInterface {
  GTypeInterface g_iface;

  /**
   * FlView::get_engine:
   * @view: an #FlView
   *
   * Gets the engine being rendered in the view.
   *
   * Returns: an #FlEngine
   */
  FlEngine* (*get_engine)(FlView* view);

  /**
   * FlView::get_id:
   * @view: an #FlView.
   *
   * Gets the Flutter view ID used by this view.
   *
   * Returns: a view ID or -1 if now ID assigned.
   */
  int64_t (*get_id)(FlView* view);

  /**
   * FlView::set_background_color:
   * @view: an #FlView.
   * @color: a background color.
   *
   * Set the background color for Flutter (defaults to black).
   */
  void (*set_background_color)(FlView* view, const GdkRGBA* color);
};

/**
 * fl_view_get_engine:
 * @view: an #FlView.
 *
 * Gets the engine being rendered in the view.
 *
 * Returns: an #FlEngine.
 */
FlEngine* fl_view_get_engine(FlView* view);

/**
 * fl_view_get_id:
 * @view: an #FlView.
 *
 * Gets the Flutter view ID used by this view.
 *
 * Returns: a view ID or -1 if now ID assigned.
 */
int64_t fl_view_get_id(FlView* view);

/**
 * fl_view_set_background_color:
 * @view: an #FlView.
 * @color: a background color.
 *
 * Set the background color for Flutter (defaults to black).
 */
void fl_view_set_background_color(FlView* view, const GdkRGBA* color);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_PUBLIC_FLUTTER_LINUX_FL_VIEW_H_
