// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_VIEW_DELEGATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_VIEW_DELEGATE_H_

#include <gdk/gdk.h>
#include <cinttypes>
#include <memory>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_key_event.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"

G_BEGIN_DECLS

G_DECLARE_INTERFACE(FlKeyboardViewDelegate,
                    fl_keyboard_view_delegate,
                    FL,
                    KEYBOARD_VIEW_DELEGATE,
                    GObject);

/**
 * FlKeyboardViewDelegate:
 *
 * TODO
 */

struct _FlKeyboardViewDelegateInterface {
  GTypeInterface g_iface;

  void (*send_key_event)(FlKeyboardViewDelegate* delegate,
                         const FlutterKeyEvent* event,
                         FlutterKeyEventCallback callback,
                         void* user_data);

  gboolean (*text_filter_key_press)(FlKeyboardViewDelegate* delegate,
                                    FlKeyEvent* event);

  FlBinaryMessenger* (*get_messenger)(FlKeyboardViewDelegate* delegate);

  void (*redispatch_event)(FlKeyboardViewDelegate* delegate,
                           std::unique_ptr<FlKeyEvent> event);
};

/**
 * fl_keyboard_view_delegate_send_key_event:
 * TODO
 */
void fl_keyboard_view_delegate_send_key_event(FlKeyboardViewDelegate* delegate,
                                              const FlutterKeyEvent* event,
                                              FlutterKeyEventCallback callback,
                                              void* user_data);

// This does not take over the ownership of `event`.
gboolean fl_keyboard_view_delegate_text_filter_key_press(
    FlKeyboardViewDelegate* delegate,
    FlKeyEvent* event);

FlBinaryMessenger* fl_keyboard_view_delegate_get_messenger(
    FlKeyboardViewDelegate* delegate);

// This will take over the ownership of `event`. Must call fl_key_event_dispose.
void fl_keyboard_view_delegate_redispatch_event(
    FlKeyboardViewDelegate* delegate,
    std::unique_ptr<FlKeyEvent> event);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_KEYBOARD_VIEW_DELEGATE_H_
