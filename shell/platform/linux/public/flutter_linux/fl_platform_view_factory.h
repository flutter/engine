// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEW_FACTORY_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEW_FACTORY_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <gtk/gtk.h>

#include "fl_platform_view.h"

G_BEGIN_DECLS

G_DECLARE_INTERFACE(FlPlatformViewFactory,
                    fl_platform_view_factory,
                    FL,
                    PLATFORM_VIEW_FACTORY,
                    GObject)

/**
 * FlPlatformViewFactory:
 *
 * #FlPlatformViewFactory vends #FlPlatformView objects for embedding.
 *
 * The following example shows how to implement an #FlPlatformViewFactory.
 * In common cases, #FlPlatformViewFactory may not hold references to your
 * own #FlPlatformView. Flutter will manage life-cycle of platform views
 * created by your factory.
 *
 * For header of your subclass:
 * |[<!-- language="C" -->
 *   // platform_view_factory.h
 *   G_DECLARE_FINAL_TYPE(WebViewFactory,
 *                        webview_factory,
 *                        WEBVIEW,
 *                        FACTORY,
 *                        GObject)
 *
 *   // Creates your own platform view factory. Messenger can be used for
 *   // Dart-side code to control single #FlPlatformView (or #GtkWidget).
 *   WebViewFactory *
 *   webview_factory_new(FlBinaryMessenger *messenger);
 * ]|
 *
 * For source of your subclass:
 * |[<!-- language="C" -->
 *   // platform_view_factory.c
 *   // Type definitions and webview_factory_new are omitted.
 *
 *   static FlPlatformView *
 *   webview_factory_create_platform_view (FlPlatformViewFactory *factory,
 *                                         int64_t view_identifier,
 *                                         FlValue *args) {
 *     // Creates a new instance of your custom #FlPlatformView implementation.
 *     // Flutter will take the reference, and release it when Dart-side widget
 *     // is destructed.
 *     WebViewFactory *self = WEBVIEW_FACTORY (factory);
 *     return FL_PLATFORM_VIEW (
 *         webview_platform_view_new (self->messenger, view_identifier, args));
 *   }
 *
 *   static FlMessageCodec *
 *   webview_factory_get_create_arguments_codec (FlPlatformViewFactory *self) {
 *     // Simply creates a new instance of message codec.
 *     return FL_MESSAGE_CODEC (fl_standard_message_codec_new ());
 *   }
 *
 *   static void
 *   webview_factory_fl_platform_view_factory_iface_init (
 *     FlPlatformViewFactoryInterface *iface) {
 *     iface->create_platform_view = webview_factory_create_platform_view;
 *     iface->get_create_arguments_codec =
 *         webview_factory_get_create_arguments_codec;
 *   }
 * ]|
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
 *        the Flutter app.
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

#endif
