// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEW_H_

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
 *
 * The following example shows how to implement an #FlPlatformView.
 * |[<!-- language="C" -->
 *   // Type definition, destructor, init and class_init are omitted.
 *   struct _WebViewPlatformView { // extends FlPlatformView
 *     FlPlatformView parent_instance;
 *     int64_t view_identifier;
 *     WebKitWebView *webview; // holds reference to your widget.
 *   };
 *
 *   G_DEFINE_TYPE(WebViewPlatformView,
 *                 webview_platform_view,
 *                 fl_platform_view_get_type ())
 *
 *   static GtkWidget *
 *   webview_plaform_view_get_view (FlPlatformView *platform_view) {
 *     // Simply return the #GtkWidget you created in constructor.
 *     // DO NOT create widgets in this function, which will be called
 *     // every frame.
 *     WebViewPlatformView *self = WEBVIEW_PLATFORM_VIEW (platform_view);
 *     return GTK_WIDGET (self->webview);
 *   }
 *
 *   // Constructor should be called by your custom #FlPlatformViewFactory.
 *   WebViewPlatformView *
 *   webview_platform_view_new (FlBinaryMessenger *messenger,
 *                              int64_t view_identifier,
 *                              FlValue *args) {
 *     // Initializes your #GtkWidget, and stores it in #FlPlatformView.
 *     WebKitWebView *webview = webview_web_view_new ();
 *
 *     WebViewPlatformView *view = WEBVIEW_PLATFORM_VIEW (
 *         g_object_new (webview_platform_view_get_type (), NULL));
 *     view->webview = webview;
 *     return view;
 *   }
 * ]|
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

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEW_H_
