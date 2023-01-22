// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_POINTER_EVENT_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_POINTER_EVENT_H_

#include <gdk/gdk.h>
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_view_private.h"

/**
 * FlPointerEventDispose:
 * @origin: the #FlPointerEvent::origin to dispose.
 *
 * The signature for #FlPointerEvent::dispose_origin, which
 * frees #FlPointerEvent::origin.
 **/
typedef void (*FlPointerEventDisposeOrigin)(gpointer origin);

/**
 * FlPointerEvent:
 * A struct that stores information from GdkEvent of either mouse or stylus.
 *
 * For why this needed: Look into doc of FlKeyEvent.
 */
typedef struct _FlPointerEvent {
  // Time in milliseconds.
  guint32 time;
  // Co-ordinates of pointer during event.
  double x, y;
  // Flutter pointer device kind.
  FlutterPointerDeviceKind fl_pointer_device_kind;
  // Pen pressure if it is a stylus.
  double pressure;
  // type of orignal event. - opaque.
  GdkEventType type;
  // button pressed if GdkEvent is GdkEventButton
  uint button;
  // GtkWindow scale factor of FlView widget.
  gint scale_factor;
  // Button modifiers.
  guint button_state;

  // An opaque pointer to the original event.
  //
  // This is used when dispatching.  For native events, this is #GdkEvent
  // pointer.  For unit tests, this is a dummy pointer.
  gpointer origin;
  // A callback to free #origin, called in #fl_pointer_event_dispose.
  //
  // Can be nullptr.
  FlPointerEventDisposeOrigin dispose_origin;
} FlPointerEvent;

/**
 * fl_pointer_event_new_from_gdk_event:
 * @event: the #GdkEvent this #FlPointerEvent is based on. The #event must be a
 * #GdkEvent, and will be destroyed by #fl_pointer_event_dispose.
 * @view: the #FLView that received this pointer event.
 *
 * Create a new #FlPointerEvent based on a #GdkEvent
 *
 * Returns: a new #FlPointerEvent. Must be freed with #fl_pointer_event_dispose.
 */
FlPointerEvent* fl_pointer_event_new_from_gdk_event(GdkEvent* event,
                                                    FlView* view);

/**
 * fl_pointer_event_dispose:
 * @event: the event to dispose.
 *
 * Properly disposes the content of #event and then the pointer.
 */
void fl_pointer_event_dispose(FlPointerEvent* event);

FlPointerEvent* fl_pointer_event_clone(const FlPointerEvent* source);

FlutterPointerDeviceKind fl_pointer_check_device_is_stylus(FlView* view,
                                                           GdkEvent* event,
                                                           double* pressure);
#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_POINTER_EVENT_H_
