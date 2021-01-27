// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEWS_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEWS_PLUGIN_H_

#include "flutter/shell/platform/linux/fl_gesture_helper.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_platform_views.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlPlatformViewsPlugin,
                     fl_platform_views_plugin,
                     FL,
                     PLATFORM_VIEWS_PLUGIN,
                     GObject);

/**
 * FlPlatformViewsPlugin:
 *
 * #FlPlatformViewPlugin is a plugin that implements the shell side
 * of SystemChannels.platform_views from the Flutter services library.
 */

/**
 * fl_platform_views_plugin_new:
 * @messenger: an #FlBinaryMessenger
 *
 * Creates a new plugin that implements SystemChannels.platform_views from the
 * Flutter services library.
 *
 * Returns: a new #FlPlatformViewsPlugin
 */
FlPlatformViewsPlugin* fl_platform_views_plugin_new(
    FlBinaryMessenger* messenger,
    FlGestureHelper* gesture_helper);

/**
 * fl_platform_views_plugin_register_view_factory:
 * @plugin: an #FlPlatformViewsPlugin.
 * @factory: the view factory that will be registered.
 * @view_type: A unique identifier for the factory, the Dart code of the Flutter
 *             app can use this identifier to request creation of a #GdkWindow
 *             by the registered factory.
 *
 * Register platform view factory
 *
 * Returns: %TRUE if view facotory has been successfully registered, %FALSE if
 * #view_type has been occupied.
 */
gboolean fl_platform_views_plugin_register_view_factory(
    FlPlatformViewsPlugin* plugin,
    FlPlatformViewFactory* factory,
    const gchar* view_type);

/**
 * fl_platform_views_plugin_get_platform_view:
 * @plugin: an #FlPlatformViewsPlugin.
 * @identifier: the identifier of platform view.
 *
 * Returns: an #FlPlatformView, or %NULL if identifier is unknown.
 */
FlPlatformView* fl_platform_views_plugin_get_platform_view(
    FlPlatformViewsPlugin* plugin,
    int identifier);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEWS_PLUGIN_H_
