// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_plugin_view_delegate.h"

G_DEFINE_INTERFACE(FlKeyboardPluginViewDelegate,
                   fl_keyboard_plugin_view_delegate,
                   G_TYPE_OBJECT)

static void fl_keyboard_plugin_view_delegate_default_init(
    FlKeyboardPluginViewDelegateInterface* iface) {}

GHashTable* fl_keyboard_plugin_view_delegate_get_keyboard_state(
    FlKeyboardPluginViewDelegate* self) {
  g_return_val_if_fail(FL_IS_KEYBOARD_PLUGIN_VIEW_DELEGATE(self), nullptr);

  return FL_KEYBOARD_PLUGIN_VIEW_DELEGATE_GET_IFACE(self)->get_keyboard_state(
      self);
}
