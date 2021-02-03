// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEWS_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEWS_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <gtk/gtk.h>

#include "fl_message_codec.h"
#include "fl_value.h"

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(FlPlatformView,
                         fl_platform_view,
                         FL,
                         PLATFORM_VIEW,
                         GObject)

/**
 * FlPlatformView:
 *
 * #FlPlatformView is an abstract class that wraps a #GtkWidget for embedding in
 * the Flutter hierarchy.
 */

struct _FlPlatformViewClass {
  GObjectClass parent_class;

  /**
   * Virtual method called when Flutter needs the #GtkWidget for embedding.
   * @platform_view: an #FlPlatformView.
   *
   * Returns: (allow-none): an #GtkWidget or %NULL if no widgets can be
   * provided.
   */
  GtkWidget* (*get_view)(FlPlatformView* platform_view);
};

/**
 * fl_platform_view_get_view:
 * @platform_view: an #FlPlatformView.
 *
 * Returns: (allow-none): an #GtkWidget or %NULL if no widgets can be provided.
 */
GtkWidget* fl_platform_view_get_view(FlPlatformView* platform_view);

//

G_DECLARE_INTERFACE(FlPlatformViewFactory,
                    fl_platform_view_factory,
                    FL,
                    PLATFORM_VIEW_FACTORY,
                    GObject)

/**
 * FlPlatformViewFactory:
 *
 * #FlPlatformViewFactory vends #FlPlatformView objects for embedding.
 */

struct _FlPlatformViewFactoryInterface {
  GTypeInterface parent_interface;

  /**
   * FlPlatformViewFactory::create_platform_view:
   * @factory: an #FlPlatformViewFactory.
   * @view_identifier: a unique identifier for this #GtkWidget.
   * @args: parameters for creating the #GtkWidget sent from the Dart side of
   *        the Flutter app. If get_create_arguments_codec is not implemented,
   *        or if no creation arguments were sent from the Dart code, this will
   *        be null. Otherwise this will be the value sent from the Dart code as
   *        decoded by get_create_arguments_codec.
   *
   * Creates an #FlPlatformView for embedding.
   *
   * Returns: (transfer full): an #FlPlatformView.
   */
  FlPlatformView* (*create_platform_view)(FlPlatformViewFactory* factory,
                                          int64_t view_identifier,
                                          FlValue* args);

  /**
   * FlPlatformViewFactory::get_create_arguments_codec:
   * @factory: an #FlPlatformViewFactory.
   *
   * Creates an #FlMessageCodec if needed. Only needs to be implemented if
   * create_platform_view needs arguments.
   *
   * Returns: (transfer full): an #FlMessageCodec for decoding the args
   * parameter of create_platform_view, or %NULL if no creation arguments will
   * be sent.
   */
  FlMessageCodec* (*get_create_arguments_codec)(FlPlatformViewFactory* self);
};

/**
 * fl_platform_view_factory_create_platform_view:
 * @factory: an #FlPlatformViewFactory.
 * @view_identifier: a unique identifier for this #GtkWidget.
 * @args: parameters for creating the #GtkWidget sent from the Dart side of
 *        the Flutter app. If get_create_arguments_codec is not implemented, or
 *        if no creation arguments were sent from the Dart code, this will be
 *        null. Otherwise this will be the value sent from the Dart code as
 *        decoded by get_create_arguments_codec.
 *
 * Creates an #FlPlatformView for embedding.
 *
 * Returns: (transfer full): an #FlPlatformView.
 */
FlPlatformView* fl_platform_view_factory_create_platform_view(
    FlPlatformViewFactory* factory,
    int64_t view_identifier,
    FlValue* args);

/**
 * fl_platform_view_factory_get_create_arguments_codec:
 * @factory: an #FlPlatformViewFactory.
 *
 * Creates an #FlMessageCodec if needed. Only needs to be implemented if
 * create_platform_view needs arguments.
 *
 * Returns: (transfer full): an #FlMessageCodec for decoding the args
 * parameter of create_platform_view, or %NULL if no creation arguments will
 * be sent.
 */
FlMessageCodec* fl_platform_view_factory_get_create_arguments_codec(
    FlPlatformViewFactory* factory);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEWS_H_
