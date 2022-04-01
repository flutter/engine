// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_PLUGIN_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_PLUGIN_REGISTRAR_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <glib-object.h>

#include "fl_binary_messenger.h"
#include "fl_platform_view_factory.h"
#include "fl_texture_registrar.h"
#include "fl_view.h"

G_BEGIN_DECLS

G_DECLARE_INTERFACE(FlPluginRegistrar,
                    fl_plugin_registrar,
                    FL,
                    PLUGIN_REGISTRAR,
                    GObject)

struct _FlPluginRegistrarInterface {
  GTypeInterface parent_iface;

  FlBinaryMessenger* (*get_messenger)(FlPluginRegistrar* registrar);

  FlTextureRegistrar* (*get_texture_registrar)(FlPluginRegistrar* registrar);

  FlView* (*get_view)(FlPluginRegistrar* registrar);
};

/**
 * FlPluginRegistrar:
 *
 * #FlPluginRegistrar is used when registering new plugins.
 */

/**
 * fl_plugin_registrar_get_messenger:
 * @registrar: an #FlPluginRegistrar.
 *
 * Gets the messenger this plugin can communicate with.
 *
 * Returns: an #FlBinaryMessenger.
 */
FlBinaryMessenger* fl_plugin_registrar_get_messenger(
    FlPluginRegistrar* registrar);

/**
 * fl_plugin_registrar_get_texture_registrar:
 * @registrar: an #FlPluginRegistrar.
 *
 * Gets the texture registrar this plugin can communicate with.
 *
 * Returns: an #FlTextureRegistrar.
 */
FlTextureRegistrar* fl_plugin_registrar_get_texture_registrar(
    FlPluginRegistrar* registrar);

/**
 * fl_plugin_registrar_get_view:
 * @registrar: an #FlPluginRegistrar.
 *
 * Get the view that Flutter is rendering with.
 *
 * Returns: (allow-none): an #FlView or %NULL if running in headless mode.
 */
FlView* fl_plugin_registrar_get_view(FlPluginRegistrar* registrar);

/**
 * fl_plugin_registrar_register_view_factory:
 * @registrar: an #FlPluginRegistrar.
 * @factory: the view factory that will be registered.
 * @view_type: A unique identifier for the factory. The Dart code of the Flutter
 *             app can use this identifier to request creation of a #GtkWidget
 *             by the registered factory.
 *
 * Registers a #FlPlatformViewFactory for creation of platform views.
 * Plugins can expose a GtkWidget for embedding in Flutter apps by registering a
 * view factory.
 */
void fl_plugin_registrar_register_view_factory(FlPluginRegistrar* registrar,
                                               FlPlatformViewFactory* factory,
                                               const gchar* view_type);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_PLUGIN_REGISTRAR_H_
