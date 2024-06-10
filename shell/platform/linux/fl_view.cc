// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

#include "flutter/shell/platform/linux/fl_view_private.h"

#include <atk-bridge.h>
#include <atk/atk.h>
#include <gtk/a11y/gtkatspi.h>

#include <cstring>

#include "flutter/shell/platform/linux/fl_accessible_application.h"
#include "flutter/shell/platform/linux/fl_backing_store_provider.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/fl_key_event.h"
#include "flutter/shell/platform/linux/fl_keyboard_manager.h"
#include "flutter/shell/platform/linux/fl_keyboard_view_delegate.h"
#include "flutter/shell/platform/linux/fl_mouse_cursor_plugin.h"
#include "flutter/shell/platform/linux/fl_platform_plugin.h"
#include "flutter/shell/platform/linux/fl_plugin_registrar_private.h"
#include "flutter/shell/platform/linux/fl_renderer_gdk.h"
#include "flutter/shell/platform/linux/fl_scrolling_manager.h"
#include "flutter/shell/platform/linux/fl_scrolling_view_delegate.h"
#include "flutter/shell/platform/linux/fl_text_input_plugin.h"
#include "flutter/shell/platform/linux/fl_text_input_view_delegate.h"
#include "flutter/shell/platform/linux/fl_view_accessible.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_engine.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_plugin_registry.h"

static constexpr int kMicrosecondsPerMillisecond = 1000;

struct _FlView {
  GtkGLArea parent_instance;

  // Project being run.
  FlDartProject* project;

  // Rendering output.
  FlRendererGdk* renderer;

  // Engine running @project.
  FlEngine* engine;

  // Pointer button state recorded for sending status updates.
  int64_t button_state;

  // Current state information for the window associated with this view.
  // GdkWindowState window_state;

  // Flutter system channel handlers.
  FlKeyboardManager* keyboard_manager;
  FlScrollingManager* scrolling_manager;
  FlTextInputPlugin* text_input_plugin;
  FlMouseCursorPlugin* mouse_cursor_plugin;
  FlPlatformPlugin* platform_plugin;

  GtkIMContext* im_context;
  GtkGesture* click_gesture;
  GtkEventController* motion_controller;
  GtkEventController* scroll_controller;
  GtkEventController* focus_controller;
  GtkEventController* key_controller;

  /* FlKeyboardViewDelegate related properties */
  KeyboardLayoutNotifier keyboard_layout_notifier;

  // Accessible tree from Flutter, exposed as an AtkPlug.
  FlViewAccessible* view_accessible;

  // Accessible socket to connect Flutter a11y to.
  GtkAccessible* socket_accessible;
};

enum { kPropFlutterProject = 1, kPropLast };

static void fl_view_plugin_registry_iface_init(
    FlPluginRegistryInterface* iface);

static void fl_view_keyboard_delegate_iface_init(
    FlKeyboardViewDelegateInterface* iface);

static void fl_view_scrolling_delegate_iface_init(
    FlScrollingViewDelegateInterface* iface);

static void fl_view_text_input_delegate_iface_init(
    FlTextInputViewDelegateInterface* iface);

G_DEFINE_TYPE_WITH_CODE(
    FlView,
    fl_view,
    GTK_TYPE_GL_AREA,
    G_IMPLEMENT_INTERFACE(fl_plugin_registry_get_type(),
                          fl_view_plugin_registry_iface_init)
        G_IMPLEMENT_INTERFACE(fl_keyboard_view_delegate_get_type(),
                              fl_view_keyboard_delegate_iface_init)
            G_IMPLEMENT_INTERFACE(fl_scrolling_view_delegate_get_type(),
                                  fl_view_scrolling_delegate_iface_init)
                G_IMPLEMENT_INTERFACE(fl_text_input_view_delegate_get_type(),
                                      fl_view_text_input_delegate_iface_init))

#if 0
// Signal handler for GtkWidget::delete-event
static gboolean window_delete_event_cb(GtkWidget* widget,
                                       GdkEvent* event,
                                       FlView* self) {
  fl_platform_plugin_request_app_exit(self->platform_plugin);
  // Stop the event from propagating.
  return TRUE;
}
#endif

// Initialize keyboard manager.
static void init_keyboard(FlView* self) {
  FlBinaryMessenger* messenger = fl_engine_get_binary_messenger(self->engine);

  g_clear_object(&self->im_context);
  self->im_context = gtk_im_multicontext_new();
  gtk_im_context_set_client_widget(self->im_context, GTK_WIDGET(self));

  g_clear_object(&self->text_input_plugin);
  self->text_input_plugin = fl_text_input_plugin_new(
      messenger, self->im_context, FL_TEXT_INPUT_VIEW_DELEGATE(self));
  g_clear_object(&self->keyboard_manager);
  self->keyboard_manager =
      fl_keyboard_manager_new(messenger, FL_KEYBOARD_VIEW_DELEGATE(self));
}

static void init_scrolling(FlView* self) {
  g_clear_object(&self->scrolling_manager);
  self->scrolling_manager =
      fl_scrolling_manager_new(FL_SCROLLING_VIEW_DELEGATE(self));
}

// Called when the engine updates accessibility.
static void update_semantics_cb(FlEngine* engine,
                                const FlutterSemanticsUpdate2* update,
                                gpointer user_data) {
  FlView* self = FL_VIEW(user_data);
  fl_view_accessible_handle_update_semantics(self->view_accessible, update);
}

// Invoked by the engine right before the engine is restarted.
//
// This method should reset states to be as if the engine had just been started,
// which usually indicates the user has requested a hot restart (Shift-R in the
// Flutter CLI.)
static void on_pre_engine_restart_cb(FlEngine* engine, gpointer user_data) {
  FlView* self = FL_VIEW(user_data);

  init_keyboard(self);
  init_scrolling(self);
}

// Implements FlPluginRegistry::get_registrar_for_plugin.
static FlPluginRegistrar* fl_view_get_registrar_for_plugin(
    FlPluginRegistry* registry,
    const gchar* name) {
  FlView* self = FL_VIEW(registry);

  return fl_plugin_registrar_new(self,
                                 fl_engine_get_binary_messenger(self->engine),
                                 fl_engine_get_texture_registrar(self->engine));
}

static void fl_view_plugin_registry_iface_init(
    FlPluginRegistryInterface* iface) {
  iface->get_registrar_for_plugin = fl_view_get_registrar_for_plugin;
}

static void fl_view_keyboard_delegate_iface_init(
    FlKeyboardViewDelegateInterface* iface) {
  iface->send_key_event =
      [](FlKeyboardViewDelegate* view_delegate, const FlutterKeyEvent* event,
         FlutterKeyEventCallback callback, void* user_data) {
        FlView* self = FL_VIEW(view_delegate);
        if (self->engine != nullptr) {
          fl_engine_send_key_event(self->engine, event, callback, user_data);
        };
      };

  iface->text_filter_key_press = [](FlKeyboardViewDelegate* view_delegate,
                                    FlKeyEvent* event) {
    FlView* self = FL_VIEW(view_delegate);
    return fl_text_input_plugin_filter_keypress(self->text_input_plugin, event);
  };

  iface->get_messenger = [](FlKeyboardViewDelegate* view_delegate) {
    FlView* self = FL_VIEW(view_delegate);
    return fl_engine_get_binary_messenger(self->engine);
  };

  iface->redispatch_event = [](FlKeyboardViewDelegate* view_delegate,
                               std::unique_ptr<FlKeyEvent> in_event) {
    FlKeyEvent* event = in_event.release();
    // This is not supported in GTK4
    fl_key_event_dispose(event);
  };

  iface->subscribe_to_layout_change = [](FlKeyboardViewDelegate* view_delegate,
                                         KeyboardLayoutNotifier notifier) {
    FlView* self = FL_VIEW(view_delegate);
    self->keyboard_layout_notifier = std::move(notifier);
  };

  iface->lookup_key = [](FlKeyboardViewDelegate* view_delegate,
                         const GdkKeymapKey* key) -> guint {
    FlView* self = FL_VIEW(view_delegate);
    int n_entries;
    g_autofree GdkKeymapKey* keys = nullptr;
    g_autofree guint* keyvals = nullptr;
    if (!gdk_display_map_keycode(gtk_widget_get_display(GTK_WIDGET(self)),
                                 key->keycode, &keys, &keyvals, &n_entries)) {
      g_printerr("lookup_key %d -> fail\n", key->keycode);
      return 0;
    }
    g_printerr("lookup_key %d -> %d (%d)\n", key->keycode, keyvals[0],
               n_entries);
    return keyvals[0];
  };

  iface->get_keyboard_state =
      [](FlKeyboardViewDelegate* view_delegate) -> GHashTable* {
    FlView* self = FL_VIEW(view_delegate);
    return fl_view_get_keyboard_state(self);
  };
}

static void fl_view_scrolling_delegate_iface_init(
    FlScrollingViewDelegateInterface* iface) {
  iface->send_mouse_pointer_event =
      [](FlScrollingViewDelegate* view_delegate, FlutterPointerPhase phase,
         size_t timestamp, double x, double y, double scroll_delta_x,
         double scroll_delta_y, int64_t buttons) {
        FlView* self = FL_VIEW(view_delegate);
        if (self->engine != nullptr) {
          fl_engine_send_mouse_pointer_event(self->engine, phase, timestamp, x,
                                             y, scroll_delta_x, scroll_delta_y,
                                             buttons);
        }
      };
  iface->send_pointer_pan_zoom_event =
      [](FlScrollingViewDelegate* view_delegate, size_t timestamp, double x,
         double y, FlutterPointerPhase phase, double pan_x, double pan_y,
         double scale, double rotation) {
        FlView* self = FL_VIEW(view_delegate);
        if (self->engine != nullptr) {
          fl_engine_send_pointer_pan_zoom_event(self->engine, timestamp, x, y,
                                                phase, pan_x, pan_y, scale,
                                                rotation);
        };
      };
}

static void fl_view_text_input_delegate_iface_init(
    FlTextInputViewDelegateInterface* iface) {
  iface->translate_coordinates = [](FlTextInputViewDelegate* delegate,
                                    gint view_x, gint view_y, gint* window_x,
                                    gint* window_y) {
    g_printerr("translate_coordinates\n");
    // FlView* self = FL_VIEW(delegate);
    // gtk_widget_translate_coordinates(GTK_WIDGET(self),
    //                                  gtk_widget_get_toplevel(GTK_WIDGET(self)),
    //                                  view_x, view_y, window_x, window_y);
    *window_x = view_x;
    *window_y = view_y;
  };
}

static void send_mouse_pointer_event(FlView* self,
                                     FlutterPointerPhase phase,
                                     guint32 timestamp,
                                     double x,
                                     double y) {
  fl_engine_send_mouse_pointer_event(self->engine, phase,
                                     timestamp * kMicrosecondsPerMillisecond, x,
                                     y, 0, 0, self->button_state);
}

static void primary_pressed_cb(FlView* self, int n_press, double x, double y) {
  self->button_state ^= kFlutterPointerButtonMousePrimary;
  send_mouse_pointer_event(self, kDown,
                           gtk_event_controller_get_current_event_time(
                               GTK_EVENT_CONTROLLER(self->click_gesture)),
                           x, y);
}

static void primary_released_cb(FlView* self, int n_press, double x, double y) {
  self->button_state ^= kFlutterPointerButtonMousePrimary;
  send_mouse_pointer_event(self, kUp,
                           gtk_event_controller_get_current_event_time(
                               GTK_EVENT_CONTROLLER(self->click_gesture)),
                           x, y);
}

static void scroll_begin_cb(FlView* self) {
  guint32 event_time =
      gtk_event_controller_get_current_event_time(self->scroll_controller);
  fl_scrolling_manager_handle_scroll_begin_event(self->scrolling_manager,
                                                 event_time);
}

static void scroll_cb(FlView* self, gdouble dx, gdouble dy) {
  guint32 event_time =
      gtk_event_controller_get_current_event_time(self->scroll_controller);
  int scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(self));
  fl_scrolling_manager_handle_scroll_event(self->scrolling_manager, event_time,
                                           dx * scale_factor,
                                           dy * scale_factor);
}

static void scroll_end_cb(FlView* self) {
  guint32 event_time =
      gtk_event_controller_get_current_event_time(self->scroll_controller);
  fl_scrolling_manager_handle_scroll_end_event(self->scrolling_manager,
                                               event_time);
}

static void enter_cb(FlView* self, gdouble x, gdouble y) {
  send_mouse_pointer_event(
      self, kAdd,
      gtk_event_controller_get_current_event_time(self->motion_controller), x,
      y);
}

static void leave_cb(FlView* self) {
  send_mouse_pointer_event(
      self, kRemove,
      gtk_event_controller_get_current_event_time(self->motion_controller), 0,
      0);
}

static void motion_cb(FlView* self, gdouble x, gdouble y) {
  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(self));
  fl_scrolling_manager_set_last_mouse_position(
      self->scrolling_manager, x * scale_factor, y * scale_factor);

  send_mouse_pointer_event(
      self, self->button_state != 0 ? kMove : kHover,
      gtk_event_controller_get_current_event_time(self->motion_controller), x,
      y);
}

static void update_window_state(FlView* self) {
  fl_engine_send_window_state_event(
      self->engine, TRUE,
      gtk_event_controller_focus_is_focus(
          GTK_EVENT_CONTROLLER_FOCUS(self->focus_controller)));
}

static void focus_changed_cb(FlView* self) {
  update_window_state(self);
}

static gboolean key_event_cb(FlView* self, GdkEvent* event) {
  GdkEventType type = gdk_event_get_event_type(event);
  if (type == GDK_KEY_PRESS || type == GDK_KEY_RELEASE) {
    return fl_keyboard_manager_handle_event(
        self->keyboard_manager, fl_key_event_new_from_gdk_event(event));
  }

  return FALSE;
}

static GdkGLContext* fl_view_create_context(GtkGLArea* gl_area) {
  FlView* self = FL_VIEW(gl_area);

  self->renderer = fl_renderer_gdk_new(
      gtk_native_get_surface(gtk_widget_get_native(GTK_WIDGET(self))));
  self->engine = fl_engine_new(self->project, FL_RENDERER(self->renderer));
  fl_engine_set_update_semantics_handler(self->engine, update_semantics_cb,
                                         self, nullptr);
  fl_engine_set_on_pre_engine_restart_handler(
      self->engine, on_pre_engine_restart_cb, self, nullptr);

  // Create system channel handlers.
  FlBinaryMessenger* messenger = fl_engine_get_binary_messenger(self->engine);
  self->mouse_cursor_plugin = fl_mouse_cursor_plugin_new(messenger, self);
  self->platform_plugin = fl_platform_plugin_new(messenger);

  g_autoptr(GError) error = nullptr;
  if (!fl_renderer_gdk_create_contexts(self->renderer, &error)) {
    gtk_gl_area_set_error(GTK_GL_AREA(self), error);
    return nullptr;
  }

  return GDK_GL_CONTEXT(
      g_object_ref(fl_renderer_gdk_get_context(self->renderer)));
}

static AtkObject* get_atk_root() {
  return ATK_OBJECT(fl_accessible_application_new());
}

static const gchar* get_atk_toolkit_name() {
  return "Flutter";
}

static const gchar* get_atk_toolkit_version() {
  return "";
}

// FIXME: Only do once
static void setup_atk() {
  AtkUtilClass* util_class = ATK_UTIL_CLASS(g_type_class_ref(ATK_TYPE_UTIL));
  util_class->get_root = get_atk_root;
  util_class->get_toolkit_name = get_atk_toolkit_name;
  util_class->get_toolkit_version = get_atk_toolkit_version;
  atk_bridge_adaptor_init(nullptr, nullptr);
  g_type_class_unref(util_class);
}

static gboolean connect_atk_plug(FlView* self) {
  self->view_accessible = fl_view_accessible_new(self->engine);

  g_autofree gchar* plug_id = atk_plug_get_id(ATK_PLUG(self->view_accessible));

  // ID is the form bus_name:path, e.g. ":1.115:/org/a11y/atspi/accessible/1"
  g_auto(GStrv) values = g_strsplit(plug_id, ":", 3);
  if (g_strv_length(values) != 3 || !g_str_equal(values[0], "")) {
    g_warning("Invalid ATK plug ID: %s", plug_id);
    return FALSE;
  }

  g_autofree gchar* bus_name = g_strdup_printf(":%s", values[1]);
  const gchar* path = values[2];
  g_autoptr(GError) error = nullptr;
  self->socket_accessible = gtk_at_spi_socket_new(bus_name, path, &error);
  if (self->socket_accessible == nullptr) {
    g_warning("Failed to make AT-SPI socket: %s", error->message);
    return FALSE;
  }

  gtk_accessible_set_accessible_parent(self->socket_accessible,
                                       GTK_ACCESSIBLE(self), NULL);

  return TRUE;
}

static void fl_view_realize(GtkWidget* widget) {
  FlView* self = FL_VIEW(widget);

  GTK_WIDGET_CLASS(fl_view_parent_class)->realize(widget);

  fl_renderer_make_current(FL_RENDERER(self->renderer));

  GError* gl_error = gtk_gl_area_get_error(GTK_GL_AREA(self));
  if (gl_error != nullptr) {
    g_warning("Failed to initialize GLArea: %s", gl_error->message);
    return;
  }

  // Handle requests by the user to close the application.
  // GtkWidget* toplevel_window = gtk_widget_get_toplevel(GTK_WIDGET(self));

  // Listen to window state changes.
  // FIXME: How to do in GTK4

  // g_signal_connect(toplevel_window, "delete-event",
  //                  G_CALLBACK(window_delete_event_cb), self);

  init_keyboard(self);

  fl_renderer_start(FL_RENDERER(FL_RENDERER(self->renderer)), self);

  g_autoptr(GError) error = nullptr;
  if (!fl_engine_start(self->engine, &error)) {
    g_warning("Failed to start Flutter engine: %s", error->message);
    return;
  }

  setup_atk();
  connect_atk_plug(self);
}

static gboolean fl_view_render(GtkGLArea* gl_area, GdkGLContext* context) {
  FlView* self = FL_VIEW(gl_area);

  if (gtk_gl_area_get_error(GTK_GL_AREA(self)) != nullptr) {
    return FALSE;
  }

  int width = gtk_widget_get_width(GTK_WIDGET(self));
  int height = gtk_widget_get_height(GTK_WIDGET(self));
  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(self));
  fl_renderer_render(FL_RENDERER(self->renderer), width * scale_factor,
                     height * scale_factor);

  return TRUE;
}

static void fl_view_resize(GtkGLArea* gl_area, int width, int height) {
  FlView* self = FL_VIEW(gl_area);

  GTK_GL_AREA_CLASS(fl_view_parent_class)->resize(gl_area, width, height);

  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(self));
  fl_engine_send_window_metrics_event(self->engine, width * scale_factor,
                                      height * scale_factor, scale_factor);
}

static void fl_view_set_property(GObject* object,
                                 guint prop_id,
                                 const GValue* value,
                                 GParamSpec* pspec) {
  FlView* self = FL_VIEW(object);

  switch (prop_id) {
    case kPropFlutterProject:
      g_set_object(&self->project,
                   static_cast<FlDartProject*>(g_value_get_object(value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void fl_view_get_property(GObject* object,
                                 guint prop_id,
                                 GValue* value,
                                 GParamSpec* pspec) {
  FlView* self = FL_VIEW(object);

  switch (prop_id) {
    case kPropFlutterProject:
      g_value_set_object(value, self->project);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void fl_view_dispose(GObject* object) {
  FlView* self = FL_VIEW(object);

  if (self->engine != nullptr) {
    fl_engine_set_update_semantics_handler(self->engine, nullptr, nullptr,
                                           nullptr);
    fl_engine_set_on_pre_engine_restart_handler(self->engine, nullptr, nullptr,
                                                nullptr);
  }

  g_clear_object(&self->project);
  g_clear_object(&self->renderer);
  g_clear_object(&self->engine);
  g_clear_object(&self->keyboard_manager);
  g_clear_object(&self->mouse_cursor_plugin);
  g_clear_object(&self->platform_plugin);
  g_clear_object(&self->view_accessible);
  g_clear_object(&self->socket_accessible);

  G_OBJECT_CLASS(fl_view_parent_class)->dispose(object);
}

static void fl_view_class_init(FlViewClass* klass) {
  GObjectClass* object_class = G_OBJECT_CLASS(klass);
  object_class->set_property = fl_view_set_property;
  object_class->get_property = fl_view_get_property;
  // object_class->notify = fl_view_notify;
  object_class->dispose = fl_view_dispose;

  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->realize = fl_view_realize;

  GtkGLAreaClass* gl_area_class = GTK_GL_AREA_CLASS(klass);
  gl_area_class->create_context = fl_view_create_context;
  gl_area_class->render = fl_view_render;
  gl_area_class->resize = fl_view_resize;

  g_object_class_install_property(
      G_OBJECT_CLASS(klass), kPropFlutterProject,
      g_param_spec_object(
          "flutter-project", "flutter-project", "Flutter project in use",
          fl_dart_project_get_type(),
          static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                   G_PARAM_STATIC_STRINGS)));
}

static void fl_view_init(FlView* self) {
  gtk_widget_set_focusable(GTK_WIDGET(self), TRUE);

  self->click_gesture = gtk_gesture_click_new();
  g_signal_connect_swapped(self->click_gesture, "pressed",
                           G_CALLBACK(primary_pressed_cb), self);
  g_signal_connect_swapped(self->click_gesture, "released",
                           G_CALLBACK(primary_released_cb), self);
  gtk_widget_add_controller(GTK_WIDGET(self),
                            GTK_EVENT_CONTROLLER(self->click_gesture));

  init_scrolling(self);
  self->scroll_controller =
      gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
  g_signal_connect_swapped(self->scroll_controller, "scroll-begin",
                           G_CALLBACK(scroll_begin_cb), self);
  g_signal_connect_swapped(self->scroll_controller, "scroll",
                           G_CALLBACK(scroll_cb), self);
  g_signal_connect_swapped(self->scroll_controller, "scroll-end",
                           G_CALLBACK(scroll_end_cb), self);
  gtk_widget_add_controller(GTK_WIDGET(self), self->scroll_controller);

  self->motion_controller = gtk_event_controller_motion_new();
  g_signal_connect_swapped(self->motion_controller, "enter",
                           G_CALLBACK(enter_cb), self);
  g_signal_connect_swapped(self->motion_controller, "leave",
                           G_CALLBACK(leave_cb), self);
  g_signal_connect_swapped(self->motion_controller, "motion",
                           G_CALLBACK(motion_cb), self);
  gtk_widget_add_controller(GTK_WIDGET(self), self->motion_controller);

  self->focus_controller = gtk_event_controller_focus_new();
  g_signal_connect_swapped(self->focus_controller, "notify::is-focus",
                           G_CALLBACK(focus_changed_cb), self);
  gtk_widget_add_controller(GTK_WIDGET(self), self->focus_controller);

  self->key_controller = gtk_event_controller_legacy_new();
  g_signal_connect_swapped(self->key_controller, "event",
                           G_CALLBACK(key_event_cb), self);
  gtk_widget_add_controller(GTK_WIDGET(self), self->key_controller);
}

G_MODULE_EXPORT FlView* fl_view_new(FlDartProject* project) {
  return static_cast<FlView*>(
      g_object_new(fl_view_get_type(), "flutter-project", project, nullptr));
}

G_MODULE_EXPORT FlEngine* fl_view_get_engine(FlView* self) {
  g_return_val_if_fail(FL_IS_VIEW(self), nullptr);
  return self->engine;
}

void fl_view_redraw(FlView* self) {
  g_return_if_fail(FL_IS_VIEW(self));
  gtk_widget_queue_draw(GTK_WIDGET(self));
}

GHashTable* fl_view_get_keyboard_state(FlView* self) {
  g_return_val_if_fail(FL_IS_VIEW(self), nullptr);
  return fl_keyboard_manager_get_pressed_state(self->keyboard_manager);
}
