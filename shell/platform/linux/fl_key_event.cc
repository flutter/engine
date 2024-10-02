// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_event.h"

FlKeyEvent* fl_key_event_new_from_gdk_event(GdkEvent* event) {
  g_return_val_if_fail(event != nullptr, nullptr);
  GdkEventType type = gdk_event_get_event_type(event);
  g_return_val_if_fail(type == GDK_KEY_PRESS || type == GDK_KEY_RELEASE,
                       nullptr);
  FlKeyEvent* result = g_new(FlKeyEvent, 1);

  guint16 keycode = 0;
  gdk_event_get_keycode(event, &keycode);
  guint keyval = 0;
  gdk_event_get_keyval(event, &keyval);
  GdkModifierType state = static_cast<GdkModifierType>(0);
  gdk_event_get_state(event, &state);

  result->time = gdk_event_get_time(event);
  result->is_press = type == GDK_KEY_PRESS;
  result->keycode = keycode;
  result->keyval = keyval;
  result->state = state;
  result->group = event->key.group;
  result->origin = event;

  return result;
}

guint32 fl_key_event_get_time(FlKeyEvent* self) {
  return self->time;
}

gboolean fl_key_event_get_is_press(FlKeyEvent* self) {
  return self->is_press;
}

guint16 fl_key_event_get_keycode(FlKeyEvent* self) {
  return self->keycode;
}

guint fl_key_event_get_keyval(FlKeyEvent* self) {
  return self->keyval;
}

GdkModifierType fl_key_event_get_state(FlKeyEvent* self) {
  return self->state;
}

guint8 fl_key_event_get_group(FlKeyEvent* self) {
  return self->group;
}

GdkEvent* fl_key_event_get_origin(FlKeyEvent* self) {
  return self->origin;
}

uint64_t fl_key_event_hash(FlKeyEvent* self) {
  // Combine the event timestamp, the type of event, and the hardware keycode
  // (scan code) of the event to come up with a unique id for this event that
  // can be derived solely from the event data itself, so that we can identify
  // whether or not we have seen this event already.
  guint64 type =
      static_cast<uint64_t>(self->is_press ? GDK_KEY_PRESS : GDK_KEY_RELEASE);
  guint64 keycode = static_cast<uint64_t>(self->keycode);
  return (self->time & 0xffffffff) | ((type & 0xffff) << 32) |
         ((keycode & 0xffff) << 48);
}

void fl_key_event_dispose(FlKeyEvent* self) {
  if (self->origin != nullptr) {
    gdk_event_free(self->origin);
  }
  g_free(self);
}
