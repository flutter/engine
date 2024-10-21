// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_TOUCH_VIEW_DELEGATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_TOUCH_VIEW_DELEGATE_H_

#include <gdk/gdk.h>
#include <cinttypes>
#include <memory>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_key_event.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"
#include "flutter/shell/platform/linux/fl_renderer.h"

G_BEGIN_DECLS

G_DECLARE_INTERFACE(FlTouchViewDelegate,
                    fl_touch_view_delegate,
                    FL,
                    TOUCH_VIEW_DELEGATE,
                    GObject);

/**
 * FlTouchViewDelegate:
 *
 * An interface for a class that provides `FlTouchManager` with
 * platform-related features.
 *
 * This interface is typically implemented by `FlView`.
 */

struct _FlTouchViewDelegateInterface {
  GTypeInterface g_iface;

  void (*send_pointer_event)(FlTouchViewDelegate* delegate,
                             const FlutterPointerEvent& event_data,
                              PointerState* state);

};

void fl_touch_view_delegate_send_pointer_event(
    FlTouchViewDelegate* delegate,
    const FlutterPointerEvent& event_data,
    PointerState* state);


G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_TOUCH_VIEW_DELEGATE_H_
