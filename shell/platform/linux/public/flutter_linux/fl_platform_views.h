// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEWS_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEWS_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <glib-object.h>
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
 * Wraps a 'GdkWindow' for embedding in the Flutter hierarchy.
 */
struct _FlPlatformViewClass {
  GObjectClass parent_class;

  /**
   * Returns a pointer to `GtkWidget`.
   */
  GtkWidget* (*get_view)(FlPlatformView* self);
};

/**
 * fl_platform_view_get_view:
 * @platform_view: an #FlPlatformView
 *
 * Returns: a pointer to #GtkWidget.
 */
GtkWidget* fl_platform_view_get_view(FlPlatformView* platform_view);

//

G_DECLARE_INTERFACE(FlPlatformViewFactory,
                    fl_platform_view_factory,
                    FL,
                    PLATFORM_VIEW_FACTORY,
                    GObject)

/**
 * Create a `FlPlatformView`.
 */
struct _FlPlatformViewFactoryInterface {
  GTypeInterface parent_interface;

  /**
   * Create a `FlPlatformView`.
   *
   * @view_identifier: A unique identifier for this `GdkWindow`.
   * @args: Parameters for creating the `GdkWindow` sent from the Dart side of
   * the Flutter app. If `createArgsCodec` is not implemented, or if no creation
   * arguments were sent from the Dart code, this will be null. Otherwise this
   * will be the value sent from the Dart code as decoded by `createArgsCodec`.
   */
  FlPlatformView* (*create_platform_view)(FlPlatformViewFactory* self,
                                          int64_t view_identifier,
                                          FlValue* args);

  /**
   * Returns the `FlMessageCodec` for decoding the args parameter of `create`.
   *
   * Only needs to be implemented if `create` needs an arguments paramter.
   */
  FlMessageCodec* (*get_create_arguments_codec)(FlPlatformViewFactory* self);
};

/**
 * Create a `FlPlatformView`.
 *
 * @factory: an #FlPlatformViewFactory.
 * @view_identifier: A unique identifier for this `GdkWindow`.
 * @args: Parameters for creating the `GdkWindow` sent from the Dart side of the
 * Flutter app. If `createArgsCodec` is not implemented, or if no creation
 * arguments were sent from the Dart code, this will be null. Otherwise this
 * will be the value sent from the Dart code as decoded by `createArgsCodec`.
 */
FlPlatformView* fl_platform_view_factory_create_platform_view(
    FlPlatformViewFactory* factory,
    int64_t view_identifier,
    FlValue* args);

/**
 * Returns the `FlMessageCodec` for decoding the args parameter of `create`.
 *
 * @factory: an #FlPlatformViewFactory.
 *
 * Only needs to be implemented if `create` needs an arguments paramter.
 */
FlMessageCodec* fl_platform_view_factory_get_create_arguments_codec(
    FlPlatformViewFactory* factory);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEWS_H_
