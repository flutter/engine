// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_gesture_helper.h"

struct _FlGestureHelper {
  GObject parent_instance;

  GList* event_list;
  GtkWidget* grabbed_widget;
  GtkWidget* hover_widget;
  int64_t pointer_id;
  int32_t pressed_buttons;
  bool end;
};

G_DEFINE_TYPE(FlGestureHelper, fl_gesture_helper, G_TYPE_OBJECT)

static void fl_gesture_helper_class_init(FlGestureHelperClass* klass) {}

static void fl_gesture_helper_init(FlGestureHelper* self) {}

FlGestureHelper* fl_gesture_helper_new() {
  return FL_GESTURE_HELPER(g_object_new(fl_gesture_helper_get_type(), nullptr));
}

static void free_event(gpointer data) {
  gdk_event_free(reinterpret_cast<GdkEvent*>(data));
}

static void send_button_event(FlGestureHelper* self,
                              GdkEventButton* event,
                              GtkWidget* widget) {
  if (event->type == GDK_BUTTON_PRESS) {
    self->end = false;
    self->pressed_buttons |= 1 << event->button;
  } else if (event->type == GDK_BUTTON_RELEASE) {
    self->end = true;
    self->pressed_buttons &= ~(1 << event->button);
  }

  GdkWindow* window = gtk_widget_get_window(widget);
  gint origin_x, origin_y;
  gdk_window_get_origin(window, &origin_x, &origin_y);
  GdkEvent button;
  button.button.type = event->type;
  g_object_ref(window);
  button.button.window = window;
  button.button.send_event = event->send_event;
  button.button.time = event->time;
  button.button.x = event->x_root - origin_x;
  button.button.y = event->y_root - origin_y;
  button.button.axes = event->axes;
  button.button.state = event->state;
  button.button.button = event->button;
  button.button.device = event->device;
  button.button.x_root = event->x_root;
  button.button.y_root = event->y_root;
  gtk_widget_event(widget, &button);
}

static void send_motion_event(FlGestureHelper* self,
                              GdkEventMotion* event,
                              GtkWidget* widget) {
  if (self->end)
    return;

  GdkWindow* window = gtk_widget_get_window(widget);
  gint origin_x, origin_y;
  gdk_window_get_origin(window, &origin_x, &origin_y);

  GdkEvent motion;
  motion.motion.type = event->type;
  g_object_ref(window);
  motion.motion.window = window;
  motion.motion.send_event = event->send_event;
  motion.motion.time = event->time;
  motion.motion.x = event->x_root - origin_x;
  motion.motion.y = event->y_root - origin_y;
  motion.motion.axes = event->axes;
  motion.motion.state = event->state;
  motion.motion.is_hint = event->is_hint;
  motion.motion.device = event->device;
  motion.motion.x_root = event->x_root;
  motion.motion.y_root = event->y_root;
  gtk_widget_event(widget, &motion);
}

static void clear_state(FlGestureHelper* self) {
  g_clear_object(&self->grabbed_widget);
  g_list_free_full(self->event_list, free_event);
  self->event_list = nullptr;
}

void fl_gesture_helper_button_press(FlGestureHelper* self, GdkEvent* event) {
  clear_state(self);
  self->pointer_id++;

  self->event_list = g_list_append(self->event_list, gdk_event_copy(event));
}

void fl_gesture_helper_button_release(FlGestureHelper* self, GdkEvent* event) {
  if (self->grabbed_widget) {
    send_button_event(self, &event->button, self->grabbed_widget);

    clear_state(self);
  } else {
    self->event_list = g_list_append(self->event_list, gdk_event_copy(event));
  }
}

void fl_gesture_helper_button_motion(FlGestureHelper* self, GdkEvent* event) {
  if (self->grabbed_widget) {
    // We have grabbed the widget, we directly distribute the event to the
    // widget.
    if (!self->end) {
      send_motion_event(self, &event->motion, self->grabbed_widget);
    }
  } else {
    if (self->pressed_buttons) {
      // Or if there are some mouse buttons pressed, delay the event.
      self->event_list = g_list_append(self->event_list, gdk_event_copy(event));
    } else if (self->hover_widget) {
      // Or mouse is hovering.
      send_motion_event(self, &event->motion, self->hover_widget);
    }
  }
}

void fl_gesture_helper_accept_gesture(FlGestureHelper* self,
                                      GtkWidget* widget,
                                      int64_t pointer_id) {
  g_set_object(&self->grabbed_widget, widget);

  // send stored events to grabbed widget.
  for (GList* event = self->event_list; event; event = event->next) {
    GdkEvent* gdk_event = reinterpret_cast<GdkEvent*>(event->data);
    if (gdk_event->type == GDK_BUTTON_PRESS ||
        gdk_event->type == GDK_BUTTON_RELEASE) {
      send_button_event(self, &gdk_event->button, self->grabbed_widget);
    } else if (gdk_event->type == GDK_MOTION_NOTIFY) {
      if (self->pressed_buttons && !self->end) {
        send_motion_event(self, &gdk_event->motion, self->grabbed_widget);
      }
    }
  }
}

void fl_gesture_helper_enter(FlGestureHelper* self, GtkWidget* widget) {
  g_set_object(&self->hover_widget, widget);
}

void fl_gesture_helper_exit(FlGestureHelper* self, GtkWidget* widget) {
  g_clear_object(&self->hover_widget);
}
