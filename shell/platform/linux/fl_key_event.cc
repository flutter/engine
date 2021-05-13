// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_event.h"

static void dispose_gdk_event(gpointer target) {
  dispose_gdk_event(reinterpret_cast<GdkEvent*>(target));
}

FlKeyEvent* fl_key_event_new_from_gdk_event(GdkEventKey* event) {
  g_return_val_if_fail(event != nullptr, nullptr);
  GdkEventType type = gdk_event_get_event_type(event);
  g_return_val_if_fail(type == GDK_KEY_PRESS || type == GDK_KEY_RELEASE, nullptr);
  bool valid = true;
  FlKeyEvent* result = g_new(FlKeyEvent, 1);

  result->time = gdk_event_get_time(event);
  result->is_press = type == GDK_KEY_PRESS;
  valid = valid && gdk_event_get_keycode(event, &result->keycode);
  valid = valid && gdk_event_get_keyval(event, &result->keyval);
  valid = valid && gdk_event_get_state(event, &result->state);
  result->origin = event;
  result->dispose_origin = dispose_gdk_event;

  g_return_val_if_fail(valid, nullptr);
  return result;
}

void fl_key_event_dispose(FlKeyEvent* event) {
  if (event->dispose_origin != nullptr) {
    event->dispose_origin(event->origin);
  }
  g_free(event);
}

void fl_key_event_destroy_notify(gpointer event) {
  fl_key_event_dispose(reinterpret_cast<FlKeyEvent*>(event));
}
