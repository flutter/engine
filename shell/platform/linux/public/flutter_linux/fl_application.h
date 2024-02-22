// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_PUBLIC_FLUTTER_LINUX_FL_APPLICATION_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_PUBLIC_FLUTTER_LINUX_FL_APPLICATION_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include "fl_view.h"

#include <gmodule.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(FlApplication, fl_application, FL, APPLICATION, GtkApplication);

struct _FlApplicationClass {
  GtkApplicationClass parent_class;
};

/**
 * FlApplication:
 *
 * #Flutter-based application with the GTK embedder. 
 *
 * Provides default behaviour for basic flutter applications.
 */

/**
 * fl_application_new:
 * @application_id: The unique identifier for the application. (Sets
 * Gio.Application:application-id)
 * @flags: Flags specifying the behaviour of the application. (Sets
 * Gio.Application:flags)
 *
 * Creates a new Flutter-based application.
 *
 * Flags should be set according to the Gio.Applicationflags. For most
 * applications this should be either G_APPLICATION_NON_UNIQUE or
 * G_APPLICATION_DEFAULT_FLAGS. (The default behavior ensures that only a
 * primary instance is open at all times. Using other flags requires special
 * care, refer to the Gio documentation for that)
 *
 * Note that properties like :handle-local-options and signals like
 * GApplication::command-line don't work. FlApplication overrides
 * Gio.Application:local-command-line().
 *
 * Returns: a new #FlApplication
 */
FlApplication* fl_application_new(const gchar* application_id,
                                  GApplicationFlags flags);

/**
 * ViewConstructedCallback:
 * @app: The #FlApplication that invoked this callback 
 * @window: the #GtkWindow embedding the #FlView 
 * @view: the constructed #FlView
 * @user_data: (closure): user-supplied data
*/
typedef void (*ViewConstructedCallback)(FlApplication* app,
                                        GtkWindow* window,
                                        FlView* view,
                                        gpointer user_data);

/**
  fl_application_connect_view_constructed:
  @c_handler: A #ViewConstructedCallback function to connect to this signal. 
  @user_data: Data to pass to the c_handler call. Can be nullptr

  It is not guranted that this is only called once, as MultiView Flutter
  applications are in development.
*/
gulong fl_application_connect_view_constructed(
    FlApplication* app,
    ViewConstructedCallback c_handler,
    gpointer user_data);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_PUBLIC_FLUTTER_LINUX_FL_APPLICATION_H_
