// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EVENT_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EVENT_H_

#include <gdk/gdk.h>

typedef void FlKeyEventDisposeOrigin(gpointer);

/**
 * FlKeyEvent:
 * A struct that stores information from GdkEvent.
 *
 * GDK will stop supporting creating GdkEvent since 4.0. This struct
 * is used so that Flutter can create an event object in unittests.
 */
typedef struct _FlKeyEvent {
  // Time in milliseconds.
  guint32 time;
  // True if is a press event, otherwise a release event.
  bool is_press;
  // Hardware keycode.
  guint16 keycode;
  // Keyval.
  guint keyval;
  // Modifier state.
  GdkModifierType state;
  // String, null-terminated.
  //
  // Can be nullptr.
  const char* string;
  // Length of #string (without the terminating \0).
  size_t length;
  // An opaque pointer to the original event.
  //
  // This is used when dispatching.
  // For native events, this is #GdkEvent pointer.
  // For unit tests, this is a dummy pointer.
  gpointer origin;
  // A callback to free #origin, called when #FlKeyEvent is disposed.
  FlKeyEventDisposeOrigin dispose_origin;
} FlKeyEvent;

/**
 * fl_key_event_new_from_gdk_event:
 * @event: the #GdkEvent this #FlKeyEvent is based on. The #event be a #GdkEventKey,
 * and will be destroyed by #fl_key_event_dispose.
 *
 * Create a new #FlKeyEvent based on a #GdkEventKey.
 *
 * Returns: a new #FlKeyEvent. Must be freed with #free (or #g_free).
 */
FlKeyEvent* fl_key_event_new_from_gdk_event(GdkEvent* event);

/**
 * fl_key_event_copy:
 * @source: the source to copy from.
 *
 * Allocates a new event with all properties shallow copied from source.
 *
 * Returns: a cloned #FlKeyEvent. Must be freed with #free (or #g_free).
 */
void fl_key_event_dispose(FlKeyEvent* event);

void fl_key_event_destroy_notify(gpointer event);

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EVENT_H_
