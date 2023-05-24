// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_pointer_event.h"

static constexpr int kMicrosecondsPerMillisecond = 1000;

static void dispose_origin_from_gdk_event(gpointer origin) {
  g_return_if_fail(origin != nullptr);
  gdk_event_free(reinterpret_cast<GdkEvent*>(origin));
}

FlPointerEvent* fl_pointer_event_new_from_gdk_event(GdkEvent* event,
                                                    FlView* view) {
  g_return_val_if_fail(event != nullptr, nullptr);
  FlPointerEvent* result = g_new(FlPointerEvent, 1);

  result->time = gdk_event_get_time(event) * kMicrosecondsPerMillisecond;
  result->type = event->type;
  if (!gdk_event_get_coords(event, &result->x, &result->y)) {
    g_warning("expected a pointer event");
    return nullptr;
  }
  result->scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(view));
  if (event->type == GDK_BUTTON_PRESS || event->type == GDK_BUTTON_RELEASE ||
      event->type == GDK_DOUBLE_BUTTON_PRESS ||
      event->type == GDK_TRIPLE_BUTTON_PRESS) {
    GdkEventButton* eventBtn = reinterpret_cast<GdkEventButton*>(event);
    result->button = eventBtn->button;
    result->button_state = eventBtn->state;
  }
  result->fl_pointer_device_kind =
      fl_pointer_check_device_is_stylus(view, event, &result->pressure);
  result->origin = event;
  result->dispose_origin = dispose_origin_from_gdk_event;

  return result;
}

void fl_pointer_event_dispose(FlPointerEvent* event) {
  if (event->dispose_origin != nullptr) {
    event->dispose_origin(event->origin);
  }
  g_free(event);
}

FlPointerEvent* fl_pointer_event_clone(const FlPointerEvent* event) {
  FlPointerEvent* new_event = g_new(FlPointerEvent, 1);
  *new_event = *event;
  return new_event;
}

FlutterPointerDeviceKind fl_pointer_check_device_is_stylus(FlView* view,
                                                           GdkEvent* event,
                                                           double* pressure) {
  GdkDevice* device = gdk_event_get_device(event);
  GdkAxisFlags flags = gdk_device_get_axes(device);
  // Setting a default value for pressure
  *pressure = 0.0;
  if (!(flags & GDK_AXIS_FLAG_PRESSURE)) {
    return kFlutterPointerDeviceKindMouse;
  }
  GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(view));
  gdouble axes[GDK_AXIS_LAST] = {
      0,
  };
  gdk_device_get_state(device, window, axes, NULL);
  gdk_device_get_axis(device, axes, GDK_AXIS_PRESSURE, pressure);
  return kFlutterPointerDeviceKindStylus;
}
