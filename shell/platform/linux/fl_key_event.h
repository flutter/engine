// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EVENT_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EVENT_H_

#include <gdk/gdk.h>

typedef struct _FlKeyEvent FlKeyEvent;

/**
 * FlKeyEventDispose:
 * @self: the self pointer to dispose.
 *
 * The signature for a callback with which a #FlKeyEvent performs
 * before being freed.
 **/
typedef void (*FlKeyEventDispose)(FlKeyEvent* self);

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
  int state;
  // String, null-terminated.
  //
  // Can be nullptr.
  const char* string;
  // Length of #string (without the terminating \0).
  size_t length;
  // An opaque pointer to the original event.
  //
  // This is used when dispatching.  For native events, this is #GdkEvent
  // pointer.  For unit tests, this is a dummy pointer.
  gpointer origin;
  // Extra steps before freeing this #FlKeyEvent.
  //
  // Usually a function that frees origin. Can be nullptr.
  FlKeyEventDispose dispose;
} FlKeyEvent;

/**
 * fl_key_event_new_from_gdk_event:
 * @event: the #GdkEvent this #FlKeyEvent is based on. The #event must be a
 * #GdkEventKey, and will be destroyed by #fl_key_event_dispose.
 *
 * Create a new #FlKeyEvent based on a #GdkEvent.
 *
 * Returns: a new #FlKeyEvent. Must be freed with #fl_key_event_dispose.
 */
FlKeyEvent* fl_key_event_new_from_gdk_event(GdkEvent* event);

/**
 * fl_key_event_dispose:
 * @event: the event to dispose.
 *
 * Properly disposes the content of #event and then the pointer.
 */
void fl_key_event_dispose(FlKeyEvent* event);

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEY_EVENT_H_
