// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_event.h"

static void dispose_from_gdk_event(FlKeyEvent* event) {
  gdk_event_free(reinterpret_cast<GdkEvent*>(event->origin));
}

FlKeyEvent* fl_key_event_new_from_gdk_event(GdkEvent* raw_event) {
  g_return_val_if_fail(raw_event != nullptr, nullptr);
  GdkEventKey* event = reinterpret_cast<GdkEventKey*>(raw_event);
  GdkEventType type = event->type;
  g_return_val_if_fail(type == GDK_KEY_PRESS || type == GDK_KEY_RELEASE,
                       nullptr);
  FlKeyEvent* result = g_new(FlKeyEvent, 1);

  result->time = event->time;
  result->is_press = type == GDK_KEY_PRESS;
  result->keycode = event->hardware_keycode;
  result->keyval = event->keyval;
  result->state = event->state;
  result->origin = event;
  result->dispose = dispose_from_gdk_event;

  return result;
}

void fl_key_event_dispose(FlKeyEvent* event) {
  if (event->dispose != nullptr) {
    event->dispose(event);
  }
  g_free(event);
}

void fl_key_event_destroy_notify(gpointer event) {
  fl_key_event_dispose(reinterpret_cast<FlKeyEvent*>(event));
}
