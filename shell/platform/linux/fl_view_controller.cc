// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_view_controller.h"

#include "flutter/shell/platform/linux/fl_view_controller_private.h"

#include <cstring>

#include "flutter/shell/platform/linux/fl_accessibility_plugin.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/fl_key_channel_responder.h"
#include "flutter/shell/platform/linux/fl_key_embedder_responder.h"
#include "flutter/shell/platform/linux/fl_key_event.h"
#include "flutter/shell/platform/linux/fl_keyboard_manager.h"
#include "flutter/shell/platform/linux/fl_mouse_cursor_plugin.h"
#include "flutter/shell/platform/linux/fl_platform_plugin.h"
#include "flutter/shell/platform/linux/fl_plugin_registrar_private.h"
#include "flutter/shell/platform/linux/fl_view_controller_accessible.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_engine.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_plugin_registry.h"

static constexpr int kMicrosecondsPerMillisecond = 1000;

struct _FlView {
  GObject parent_instance;

  // Project being run.
  FlDartProject* project;

  // Rendering output.
  FlRenderer* renderer;

  // Engine running @project.
  FlEngine* engine;

  // Pointer button state recorded for sending status updates.
  int64_t button_state;

  // Flutter system channel handlers.
  FlAccessibilityPlugin* accessibility_plugin;
  FlKeyboardManager* keyboard_manager;
  FlMouseCursorPlugin* mouse_cursor_plugin;
  FlPlatformPlugin* platform_plugin;

  GList* gl_area_list;
  GList* used_area_list;

  GList* children_list;
  GList* pending_children_list;

  // Tracks whether mouse pointer is inside the view.
  gboolean pointer_inside;
};

enum { PROP_FLUTTER_PROJECT = 1, PROP_LAST };

static void fl_view_controller_plugin_registry_iface_init(
    FlPluginRegistryInterface* iface);

static gboolean text_input_im_filter_by_gtk(GtkIMContext* im_context,
                                            gpointer gdk_event);

static void redispatch_key_event_by_gtk(gpointer gdk_event);

G_DEFINE_TYPE_WITH_CODE(
    FlViewController,
    fl_view_controller,
    GTK_TYPE_CONTAINER,
    G_IMPLEMENT_INTERFACE(fl_plugin_registry_get_type(),
                          fl_view_controller_plugin_registry_iface_init))

static void fl_view_controller_update_semantics_node_cb(FlEngine* engine,
                                             const FlutterSemanticsNode* node,
                                             gpointer user_data) {
  FlViewController* self = FL_VIEW(user_data);

  fl_accessibility_plugin_handle_update_semantics_node(
      self->accessibility_plugin, node);
}

static void fl_view_controller_init_keyboard(FlViewController* self) {
  FlBinaryMessenger* messenger = fl_engine_get_binary_messenger(self->engine);
  self->keyboard_manager = fl_keyboard_manager_new(
      fl_text_input_plugin_new(messenger, self, text_input_im_filter_by_gtk),
      redispatch_key_event_by_gtk);
  // The embedder responder must be added before the channel responder.
  fl_keyboard_manager_add_responder(
      self->keyboard_manager,
      FL_KEY_RESPONDER(fl_key_embedder_responder_new(self->engine)));
  fl_keyboard_manager_add_responder(
      self->keyboard_manager,
      FL_KEY_RESPONDER(fl_key_channel_responder_new(messenger)));
}

// Called when the engine is restarted.
//
// This method should reset states to as if the engine has just been started,
// which usually indicates the user has requested a hot restart (Shift-R in the
// Flutter CLI.)
static void fl_view_controller_on_pre_engine_restart_cb(FlEngine* engine,
                                             gpointer user_data) {
  FlViewController* self = FL_VIEW(user_data);

  g_clear_object(&self->keyboard_manager);
  fl_view_controller_init_keyboard(self);
}

// Converts a GDK button event into a Flutter event and sends it to the engine.
static gboolean fl_view_controller_send_pointer_button_event(FlViewController* self,
                                                  GdkEventButton* event) {
  int64_t button;
  switch (event->button) {
    case 1:
      button = kFlutterPointerButtonMousePrimary;
      break;
    case 2:
      button = kFlutterPointerButtonMouseMiddle;
      break;
    case 3:
      button = kFlutterPointerButtonMouseSecondary;
      break;
    default:
      return FALSE;
  }
  int old_button_state = self->button_state;
  FlutterPointerPhase phase = kMove;
  if (event->type == GDK_BUTTON_PRESS) {
    // Drop the event if Flutter already thinks the button is down.
    if ((self->button_state & button) != 0) {
      return FALSE;
    }
    self->button_state ^= button;

    phase = old_button_state == 0 ? kDown : kMove;
  } else if (event->type == GDK_BUTTON_RELEASE) {
    // Drop the event if Flutter already thinks the button is up.
    if ((self->button_state & button) == 0) {
      return FALSE;
    }
    self->button_state ^= button;

    phase = self->button_state == 0 ? kUp : kMove;
  }

  if (self->engine == nullptr) {
    return FALSE;
  }

  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(self));
  fl_engine_send_mouse_pointer_event(
      self->engine, phase, event->time * kMicrosecondsPerMillisecond,
      event->x * scale_factor, event->y * scale_factor, 0, 0,
      self->button_state);

  return TRUE;
}

// Implements FlPluginRegistry::get_registrar_for_plugin.
static FlPluginRegistrar* fl_view_controller_get_registrar_for_plugin(
    FlPluginRegistry* registry,
    const gchar* name) {
  FlViewController* self = FL_VIEW(registry);

  return fl_plugin_registrar_new(self,
                                 fl_engine_get_binary_messenger(self->engine),
                                 fl_engine_get_texture_registrar(self->engine));
}

static void fl_view_controller_plugin_registry_iface_init(
    FlPluginRegistryInterface* iface) {
  iface->get_registrar_for_plugin = fl_view_controller_get_registrar_for_plugin;
}

static gboolean event_box_button_release_event(GtkWidget* widget,
                                               GdkEventButton* event,
                                               FlViewController* view);

static gboolean event_box_button_press_event(GtkWidget* widget,
                                             GdkEventButton* event,
                                             FlViewController* view);

static gboolean event_box_scroll_event(GtkWidget* widget,
                                       GdkEventScroll* event,
                                       FlViewController* view);

static gboolean event_box_motion_notify_event(GtkWidget* widget,
                                              GdkEventMotion* event,
                                              FlViewController* view);

static gboolean event_box_enter_notify_event(GtkWidget* widget,
                                             GdkEventCrossing* event,
                                             FlViewController* view);

static gboolean event_box_leave_notify_event(GtkWidget* widget,
                                             GdkEventCrossing* event,
                                             FlViewController* view);

static void fl_view_controller_constructed(GObject* object) {
  FlViewController* self = FL_VIEW(object);

  self->renderer = FL_RENDERER(fl_renderer_gl_new());
  self->engine = fl_engine_new(self->project, self->renderer);
  fl_engine_set_update_semantics_node_handler(
      self->engine, fl_view_controller_update_semantics_node_cb, self, nullptr);
  fl_engine_set_on_pre_engine_restart_handler(
      self->engine, fl_view_controller_on_pre_engine_restart_cb, self, nullptr);

  // Create system channel handlers.
  FlBinaryMessenger* messenger = fl_engine_get_binary_messenger(self->engine);
  self->accessibility_plugin = fl_accessibility_plugin_new(self);
  fl_view_controller_init_keyboard(self);
  self->mouse_cursor_plugin = fl_mouse_cursor_plugin_new(messenger, self);
  self->platform_plugin = fl_platform_plugin_new(messenger);

  self->event_box = gtk_event_box_new();
  gtk_widget_set_parent(self->event_box, GTK_WIDGET(self));
  gtk_widget_show(self->event_box);
  gtk_widget_add_events(self->event_box,
                        GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK |
                            GDK_SMOOTH_SCROLL_MASK);

  g_signal_connect(self->event_box, "button-press-event",
                   G_CALLBACK(event_box_button_press_event), self);
  g_signal_connect(self->event_box, "button-release-event",
                   G_CALLBACK(event_box_button_release_event), self);
  g_signal_connect(self->event_box, "scroll-event",
                   G_CALLBACK(event_box_scroll_event), self);
  g_signal_connect(self->event_box, "motion-notify-event",
                   G_CALLBACK(event_box_motion_notify_event), self);
  g_signal_connect(self->event_box, "enter-notify-event",
                   G_CALLBACK(event_box_enter_notify_event), self);
  g_signal_connect(self->event_box, "leave-notify-event",
                   G_CALLBACK(event_box_leave_notify_event), self);
}

static void fl_view_controller_set_property(GObject* object,
                                 guint prop_id,
                                 const GValue* value,
                                 GParamSpec* pspec) {
  FlViewController* self = FL_VIEW(object);

  switch (prop_id) {
    case PROP_FLUTTER_PROJECT:
      g_set_object(&self->project,
                   static_cast<FlDartProject*>(g_value_get_object(value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void fl_view_controller_get_property(GObject* object,
                                 guint prop_id,
                                 GValue* value,
                                 GParamSpec* pspec) {
  FlViewController* self = FL_VIEW(object);

  switch (prop_id) {
    case PROP_FLUTTER_PROJECT:
      g_value_set_object(value, self->project);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void fl_view_controller_notify(GObject* object, GParamSpec* pspec) {
  FlViewController* self = FL_VIEW(object);

  if (strcmp(pspec->name, "scale-factor") == 0) {
    fl_view_controller_geometry_changed(self);
  }

  if (G_OBJECT_CLASS(fl_view_controller_parent_class)->notify != nullptr) {
    G_OBJECT_CLASS(fl_view_controller_parent_class)->notify(object, pspec);
  }
}

static void fl_view_controller_dispose(GObject* object) {
  FlViewController* self = FL_VIEW(object);

  if (self->engine != nullptr) {
    fl_engine_set_update_semantics_node_handler(self->engine, nullptr, nullptr,
                                                nullptr);
    fl_engine_set_on_pre_engine_restart_handler(self->engine, nullptr, nullptr,
                                                nullptr);
  }

  g_clear_object(&self->project);
  g_clear_object(&self->renderer);
  g_clear_object(&self->engine);
  g_clear_object(&self->accessibility_plugin);
  g_clear_object(&self->keyboard_manager);
  g_clear_object(&self->mouse_cursor_plugin);
  g_clear_object(&self->platform_plugin);

  G_OBJECT_CLASS(fl_view_controller_parent_class)->dispose(object);
}

// Implements GtkWidget::realize.
static void fl_view_controller_realize(GtkWidget* widget) {
  FlViewController* self = FL_VIEW(widget);
  g_autoptr(GError) error = nullptr;

  if (!fl_renderer_start(self->renderer, self, &error)) {
    g_warning("Failed to start Flutter renderer: %s", error->message);
    return;
  }

  if (!fl_engine_start(self->engine, &error)) {
    g_warning("Failed to start Flutter engine: %s", error->message);
    return;
  }
}

static gboolean event_box_button_press_event(GtkWidget* widget,
                                             GdkEventButton* event,
                                             FlViewController* view) {
  // Flutter doesn't handle double and triple click events.
  if (event->type == GDK_DOUBLE_BUTTON_PRESS ||
      event->type == GDK_TRIPLE_BUTTON_PRESS) {
    return FALSE;
  }

  if (!gtk_widget_has_focus(GTK_WIDGET(view))) {
    gtk_widget_grab_focus(GTK_WIDGET(view));
  }

  return fl_view_controller_send_pointer_button_event(view, event);
}

static gboolean event_box_button_release_event(GtkWidget* widget,
                                               GdkEventButton* event,
                                               FlViewController* view) {
  return fl_view_controller_send_pointer_button_event(view, event);
}

static gboolean event_box_scroll_event(GtkWidget* widget,
                                       GdkEventScroll* event,
                                       FlViewController* view) {
  // TODO(robert-ancell): Update to use GtkEventControllerScroll when we can
  // depend on GTK 3.24.

  gdouble scroll_delta_x = 0.0, scroll_delta_y = 0.0;
  if (event->direction == GDK_SCROLL_SMOOTH) {
    scroll_delta_x = event->delta_x;
    scroll_delta_y = event->delta_y;
  } else if (event->direction == GDK_SCROLL_UP) {
    scroll_delta_y = -1;
  } else if (event->direction == GDK_SCROLL_DOWN) {
    scroll_delta_y = 1;
  } else if (event->direction == GDK_SCROLL_LEFT) {
    scroll_delta_x = -1;
  } else if (event->direction == GDK_SCROLL_RIGHT) {
    scroll_delta_x = 1;
  }

  // The multiplier is taken from the Chromium source
  // (ui/events/x/events_x_utils.cc).
  const int kScrollOffsetMultiplier = 53;
  scroll_delta_x *= kScrollOffsetMultiplier;
  scroll_delta_y *= kScrollOffsetMultiplier;

  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(view));
  fl_engine_send_mouse_pointer_event(
      view->engine, view->button_state != 0 ? kMove : kHover,
      event->time * kMicrosecondsPerMillisecond, event->x * scale_factor,
      event->y * scale_factor, scroll_delta_x, scroll_delta_y,
      view->button_state);

  return TRUE;
}

static void check_pointer_inside(FlViewController* view, GdkEvent* event) {
  if (!view->pointer_inside) {
    view->pointer_inside = TRUE;

    gdouble x, y;
    if (gdk_event_get_coords(event, &x, &y)) {
      gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(view));

      fl_engine_send_mouse_pointer_event(
          view->engine, kAdd,
          gdk_event_get_time(event) * kMicrosecondsPerMillisecond,
          x * scale_factor, y * scale_factor, 0, 0, view->button_state);
    }
  }
}

static gboolean event_box_motion_notify_event(GtkWidget* widget,
                                              GdkEventMotion* event,
                                              FlViewController* view) {
  if (view->engine == nullptr) {
    return FALSE;
  }

  check_pointer_inside(view, reinterpret_cast<GdkEvent*>(event));

  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(view));
  fl_engine_send_mouse_pointer_event(
      view->engine, view->button_state != 0 ? kMove : kHover,
      event->time * kMicrosecondsPerMillisecond, event->x * scale_factor,
      event->y * scale_factor, 0, 0, view->button_state);

  return TRUE;
}

static gboolean event_box_enter_notify_event(GtkWidget* widget,
                                             GdkEventCrossing* event,
                                             FlViewController* view) {
  if (view->engine == nullptr) {
    return FALSE;
  }

  check_pointer_inside(view, reinterpret_cast<GdkEvent*>(event));

  return TRUE;
}

static gboolean event_box_leave_notify_event(GtkWidget* widget,
                                             GdkEventCrossing* event,
                                             FlViewController* view) {
  if (view->engine == nullptr) {
    return FALSE;
  }

  // Don't remove pointer while button is down; In case of dragging outside of
  // window with mouse grab active Gtk will send another leave notify on
  // release.
  if (view->pointer_inside && view->button_state == 0) {
    gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(view));
    fl_engine_send_mouse_pointer_event(
        view->engine, kRemove, event->time * kMicrosecondsPerMillisecond,
        event->x * scale_factor, event->y * scale_factor, 0, 0,
        view->button_state);
    view->pointer_inside = FALSE;
  }

  return TRUE;
}

// Implements GtkWidget::key_press_event.
static gboolean fl_view_controller_key_press_event(GtkWidget* widget, GdkEventKey* event) {
  FlViewController* self = FL_VIEW(widget);

  return fl_keyboard_manager_handle_event(
      self->keyboard_manager, fl_key_event_new_from_gdk_event(gdk_event_copy(
                                  reinterpret_cast<GdkEvent*>(event))));
}

// Implements GtkWidget::key_release_event.
static gboolean fl_view_controller_key_release_event(GtkWidget* widget,
                                          GdkEventKey* event) {
  FlViewController* self = FL_VIEW(widget);
  return fl_keyboard_manager_handle_event(
      self->keyboard_manager, fl_key_event_new_from_gdk_event(gdk_event_copy(
                                  reinterpret_cast<GdkEvent*>(event))));
}

static void fl_view_controller_class_init(FlViewClass* klass) {
  GObjectClass* object_class = G_OBJECT_CLASS(klass);
  object_class->constructed = fl_view_controller_constructed;
  object_class->set_property = fl_view_controller_set_property;
  object_class->get_property = fl_view_controller_get_property;
  object_class->notify = fl_view_controller_notify;
  object_class->dispose = fl_view_controller_dispose;

  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->realize = fl_view_controller_realize;
  widget_class->key_press_event = fl_view_controller_key_press_event;
  widget_class->key_release_event = fl_view_controller_key_release_event;

  g_object_class_install_property(
      G_OBJECT_CLASS(klass), PROP_FLUTTER_PROJECT,
      g_param_spec_object(
          "flutter-project", "flutter-project", "Flutter project in use",
          fl_dart_project_get_type(),
          static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                   G_PARAM_STATIC_STRINGS)));
}

G_MODULE_EXPORT FlViewController* fl_view_controller_new(FlDartProject* project) {
  return static_cast<FlViewController*>(
      g_object_new(fl_view_controller_get_type(), "flutter-project", project, nullptr));
}

G_MODULE_EXPORT FlEngine* fl_view_controller_get_engine(FlViewController* view) {
  g_return_val_if_fail(FL_IS_VIEW(view), nullptr);
  return view->engine;
}

static void redispatch_key_event_by_gtk(gpointer raw_event) {
  GdkEvent* gdk_event = reinterpret_cast<GdkEvent*>(raw_event);
  GdkEventType type = gdk_event->type;
  g_return_if_fail(type == GDK_KEY_PRESS || type == GDK_KEY_RELEASE);
  gdk_event_put(gdk_event);
}

static gboolean text_input_im_filter_by_gtk(GtkIMContext* im_context,
                                            gpointer gdk_event) {
  GdkEventKey* event = reinterpret_cast<GdkEventKey*>(gdk_event);
  GdkEventType type = event->type;
  g_return_val_if_fail(type == GDK_KEY_PRESS || type == GDK_KEY_RELEASE, false);
  return gtk_im_context_filter_keypress(im_context, event);
}
