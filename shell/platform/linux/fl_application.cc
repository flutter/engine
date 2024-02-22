// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_application.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_dart_project.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

#include <gmodule.h>
#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

struct FlApplicationPrivate {
  char** dart_entrypoint_arguments;
  gboolean activated;
};
#define FL_APPLICATION_PRIVATE(val) static_cast<FlApplicationPrivate*>(val)

enum {
    VIEW_CONSTRUCTED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE_WITH_CODE(FlApplication,
                        fl_application,
                        GTK_TYPE_APPLICATION,
                        G_ADD_PRIVATE(FlApplication))

// Implements GApplication::activate.
static void fl_application_activate(GApplication* application) {
  FlApplication* self = FL_APPLICATION(application);
  FlApplicationPrivate* priv =
      FL_APPLICATION_PRIVATE(fl_application_get_instance_private(self));
  
  GtkApplicationWindow* app_window =
      GTK_APPLICATION_WINDOW(gtk_application_window_new(GTK_APPLICATION(self)));
  GtkWindow* window = GTK_WINDOW(app_window);

  // Ensure we only run this function once. We currently don't do anything fancy
  // like signaling the application that it should show itself again. 
  // This only stops the same instance of GApplication creating multiple
  // windows/views.
  if (priv->activated) {
    return;
  }
  priv->activated = TRUE;

  // Use a header bar when running in GNOME as this is the common style used
  // by applications and is the setup most users will be using (e.g. Ubuntu
  // desktop).
  // If running on X and not using GNOME then just use a traditional title bar
  // in case the window manager does more exotic layout, e.g. tiling.
  // If running on Wayland assume the header bar will work (may need changing
  // if future cases occur).
  gboolean use_header_bar = TRUE;
#ifdef GDK_WINDOWING_X11
  GdkScreen* screen = gtk_window_get_screen(window);
  if (GDK_IS_X11_SCREEN(screen)) {
    const gchar* wm_name = gdk_x11_screen_get_window_manager_name(screen);
    if (g_strcmp0(wm_name, "GNOME Shell") != 0) {
      use_header_bar = FALSE;
    }
  }
#endif
  if (use_header_bar) {
    GtkHeaderBar* header_bar = GTK_HEADER_BAR(gtk_header_bar_new());
    gtk_header_bar_set_show_close_button(header_bar, true);
    gtk_widget_show(GTK_WIDGET(header_bar));

    gtk_window_set_titlebar(window, GTK_WIDGET(header_bar));
  }

  g_autoptr(FlDartProject) project = fl_dart_project_new();
  fl_dart_project_set_dart_entrypoint_arguments(
      project, priv->dart_entrypoint_arguments);

  FlView* view = fl_view_new(project);
  gtk_widget_show(GTK_WIDGET(view));
  gtk_container_add(GTK_CONTAINER(app_window), GTK_WIDGET(view));

  gtk_widget_grab_focus(GTK_WIDGET(view));

  g_signal_emit (G_OBJECT (self),
                 signals[VIEW_CONSTRUCTED],
                 0,
                 window, view, nullptr);
}

// Implements GApplication::local_command_line.
static gboolean fl_application_local_command_line(GApplication* application,
                                                  gchar*** arguments,
                                                  int* exit_status) {
  FlApplication* self = FL_APPLICATION(application);
  FlApplicationPrivate* priv =
      FL_APPLICATION_PRIVATE(fl_application_get_instance_private(self));

  // Strip out the first argument as it is the binary name.
  priv->dart_entrypoint_arguments = g_strdupv(*arguments + 1);

  g_autoptr(GError) error = nullptr;
  if (!g_application_register(application, nullptr, &error)) {
     g_warning("Failed to register: %s", error->message);
     *exit_status = 1;
     return TRUE;
  }

  // This will only run on the primary instance or this instance with
  // G_APPLICATION_NON_UNIQUE
  g_application_activate(application);
  *exit_status = 0;

  return TRUE;
}

// Implements GObject::dispose.
static void fl_application_dispose(GObject* object) {
  FlApplication* self = FL_APPLICATION(object);
  FlApplicationPrivate* priv =
      FL_APPLICATION_PRIVATE(fl_application_get_instance_private(self));

  g_clear_pointer(&priv->dart_entrypoint_arguments, g_strfreev);
  
  G_OBJECT_CLASS(fl_application_parent_class)->dispose(object);
}

static void fl_application_class_init(FlApplicationClass* klass) {
  G_APPLICATION_CLASS(klass)->activate = fl_application_activate;
  G_APPLICATION_CLASS(klass)->local_command_line = fl_application_local_command_line;
  G_OBJECT_CLASS(klass)->dispose = fl_application_dispose;

  /**
   * FlApplication::view-constructed:
   * @application: the #FlApplication which emitted the signal
   * @window: the created #GtkWindow with the FlView embedded
   * @view: the created #FlView
   * 
   * Emitted when a FlView with its GtkWindow is created by FlApplication.
   * Applications can connect to this signal to register plugins and setup 
   * communication with the Flutter Engine.
   * 
   * Also see #fl_application_connect_view_constructed for a more type-safe
   * g_signal_connect wrapper for this signal.
   */
  signals[VIEW_CONSTRUCTED] = 
    g_signal_new ("view-constructed",
                 G_TYPE_FROM_CLASS (klass),
                 static_cast<GSignalFlags>(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
                 0 /* closure */,
                 nullptr /* accumulator */,
                 nullptr /* accumulator data */,
                 nullptr /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 2 /* n_params */,
                 /* param_types */
                 GTK_TYPE_WINDOW,
                  fl_view_get_type());
}

static void fl_application_init(FlApplication* self) {
  FlApplicationPrivate* priv =
      FL_APPLICATION_PRIVATE(fl_application_get_instance_private(self));

  priv->activated = FALSE;
  priv->dart_entrypoint_arguments = nullptr;
}

G_MODULE_EXPORT
FlApplication* fl_application_new(const gchar* application_id,
                                  GApplicationFlags flags) {
  return FL_APPLICATION(g_object_new(fl_application_get_type(),
                                     "application-id", application_id,
                                     "flags", flags,
                                     nullptr));
}

G_MODULE_EXPORT
gulong fl_application_connect_view_constructed(
    FlApplication* app,
    ViewConstructedCallback c_handler,
    gpointer user_data) {
  g_assert(FL_IS_APPLICATION(app));

  return g_signal_connect (app, "view-constructed", G_CALLBACK(c_handler), user_data);
}
