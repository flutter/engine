// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_gesture_helper.h"

struct _FlGestureHelper {
  GObject parent_instance;

  GList* event_list;
  GtkWidget* grabbed_widget;
  int64_t pointer_id;
  bool end;
};

G_DEFINE_TYPE(FlGestureHelper, fl_gesture_helper, G_TYPE_OBJECT)

static void fl_gesture_helper_class_init(FlGestureHelperClass* klass) {}

static void fl_gesture_helper_init(FlGestureHelper* gesture_helper) {}

FlGestureHelper* fl_gesture_helper_new() {
  return FL_GESTURE_HELPER(g_object_new(fl_gesture_helper_get_type(), nullptr));
}

static void free_event(gpointer data) {
  gdk_event_free(reinterpret_cast<GdkEvent*>(data));
}

static void send_button_event(FlGestureHelper* self, GdkEventButton* event) {
  GtkWidget* widget = self->grabbed_widget;
  int64_t pointer_id = self->pointer_id;

  if (event->type == GDK_BUTTON_PRESS) {
    self->end = false;
  } else {
    self->end = true;
  }

  GdkWindow* window = gtk_widget_get_window(widget);
  gint origin_x, origin_y;
  gdk_window_get_origin(window, &origin_x, &origin_y);
  GdkEvent touch;
  touch.touch.type = event->type == GDK_BUTTON_PRESS     ? GDK_TOUCH_BEGIN
                     : event->type == GDK_BUTTON_RELEASE ? GDK_TOUCH_END
                                                         : GDK_TOUCH_CANCEL;
  g_object_ref(window);
  touch.touch.window = window;
  touch.touch.send_event = event->send_event;
  touch.touch.time = event->time;
  touch.touch.x = event->x_root - origin_x;
  touch.touch.y = event->y_root - origin_y;
  touch.touch.axes = event->axes;
  touch.touch.state = event->state;
  touch.touch.sequence =
      reinterpret_cast<GdkEventSequence*>(GINT_TO_POINTER(pointer_id));
  touch.touch.emulating_pointer = TRUE;
  touch.touch.device = event->device;
  touch.touch.x_root = event->x_root;
  touch.touch.y_root = event->y_root;
  gtk_widget_event(widget, &touch);
}

static void send_motion_event(FlGestureHelper* self, GdkEventMotion* event) {
  GtkWidget* widget = self->grabbed_widget;
  int64_t pointer_id = self->pointer_id;

  if (self->end)
    return;

  GdkWindow* window = gtk_widget_get_window(widget);
  gint origin_x, origin_y;
  gdk_window_get_origin(window, &origin_x, &origin_y);

  GdkEvent touch;
  touch.touch.type = GDK_TOUCH_UPDATE;
  g_object_ref(window);
  touch.touch.window = window;
  touch.touch.send_event = event->send_event;
  touch.touch.time = event->time;
  touch.touch.x = event->x_root - origin_x;
  touch.touch.y = event->y_root - origin_y;
  touch.touch.axes = event->axes;
  touch.touch.state = event->state;
  touch.touch.sequence =
      reinterpret_cast<GdkEventSequence*>(GINT_TO_POINTER(pointer_id));
  touch.touch.emulating_pointer = TRUE;
  touch.touch.device = event->device;
  touch.touch.x_root = event->x_root;
  touch.touch.y_root = event->y_root;
  gtk_widget_event(widget, &touch);
}

static void clear_state(FlGestureHelper* gesture_helper) {
  g_clear_object(&gesture_helper->grabbed_widget);
  g_list_free_full(gesture_helper->event_list, free_event);
  gesture_helper->event_list = nullptr;
}

void fl_gesture_helper_button_press(FlGestureHelper* gesture_helper,
                                    GdkEvent* event) {
  clear_state(gesture_helper);
  gesture_helper->pointer_id++;

  gesture_helper->event_list =
      g_list_append(gesture_helper->event_list, gdk_event_copy(event));
}

void fl_gesture_helper_button_release(FlGestureHelper* gesture_helper,
                                      GdkEvent* event) {
  if (gesture_helper->grabbed_widget) {
    send_button_event(gesture_helper, &event->button);

    clear_state(gesture_helper);
  } else {
    gesture_helper->event_list =
        g_list_append(gesture_helper->event_list, gdk_event_copy(event));
  }
}

void fl_gesture_helper_button_motion(FlGestureHelper* gesture_helper,
                                     GdkEvent* event) {
  if (gesture_helper->grabbed_widget) {
    if (!gesture_helper->end) {
      send_motion_event(gesture_helper, &event->motion);
    }
  } else {
    gesture_helper->event_list =
        g_list_append(gesture_helper->event_list, gdk_event_copy(event));
  }
}

void fl_gesture_helper_accept_gesture(FlGestureHelper* gesture_helper,
                                      GtkWidget* widget,
                                      int64_t pointer_id) {
  g_set_object(&gesture_helper->grabbed_widget, widget);

  // send stored events to grabbed widget.
  for (GList* event = gesture_helper->event_list; event; event = event->next) {
    GdkEvent* gdk_event = reinterpret_cast<GdkEvent*>(event->data);
    if (gdk_event->type == GDK_BUTTON_PRESS ||
        gdk_event->type == GDK_BUTTON_RELEASE) {
      send_button_event(gesture_helper, &gdk_event->button);
    } else if (gdk_event->type == GDK_MOTION_NOTIFY) {
      if (!gesture_helper->end) {
        send_motion_event(gesture_helper, &gdk_event->motion);
      }
    }
  }
}
