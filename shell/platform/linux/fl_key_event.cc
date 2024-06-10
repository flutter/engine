// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_event.h"

FlKeyEvent* fl_key_event_new_from_gdk_event(GdkEvent* event) {
  g_return_val_if_fail(event != nullptr, nullptr);
  GdkEventType type = gdk_event_get_event_type(event);
  g_return_val_if_fail(type == GDK_KEY_PRESS || type == GDK_KEY_RELEASE,
                       nullptr);
  FlKeyEvent* result = g_new0(FlKeyEvent, 1);

  result->time = gdk_event_get_time(event);
  result->is_press = type == GDK_KEY_PRESS;
  result->keycode = gdk_key_event_get_keycode(event);
  result->keyval = gdk_key_event_get_keyval(event);
  result->state = gdk_event_get_modifier_state(event);
  result->group = gdk_key_event_get_layout(event);
  result->origin = gdk_event_ref(event);

  return result;
}

void fl_key_event_dispose(FlKeyEvent* event) {
  if (event->origin != nullptr) {
    gdk_event_unref(event->origin);
  }
  g_free(event);
}

FlKeyEvent* fl_key_event_clone(const FlKeyEvent* event) {
  FlKeyEvent* new_event = g_new(FlKeyEvent, 1);
  *new_event = *event;
  return new_event;
}
