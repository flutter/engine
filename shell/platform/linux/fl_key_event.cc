// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_event.h"

FlKeyEvent* fl_key_event_new_from_gdk_event(GdkEventKey* event) {
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

  g_return_val_if_fail(valid, nullptr);
  return result;
}

FlKeyEvent* fl_key_event_copy(FlKeyEvent* source) {
  FlKeyEvent* result = g_new(FlKeyEvent, 1);
  *result = *source;
  return result;
}
