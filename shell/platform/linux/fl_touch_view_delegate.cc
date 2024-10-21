// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_touch_view_delegate.h"

G_DEFINE_INTERFACE(FlTouchViewDelegate,
                   fl_touch_view_delegate,
                   G_TYPE_OBJECT)

static void fl_touch_view_delegate_default_init(
    FlTouchViewDelegateInterface* iface) {}

void fl_touch_view_delegate_send_pointer_event(
    FlTouchViewDelegate* self,
    const FlutterPointerEvent& event_data,
    PointerState* state) {
  g_return_if_fail(FL_IS_TOUCH_VIEW_DELEGATE(self));

  FL_TOUCH_VIEW_DELEGATE_GET_IFACE(self)->send_pointer_event(self, event_data,
                                                             state);
}



