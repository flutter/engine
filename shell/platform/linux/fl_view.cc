// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

#include <gmodule.h>

G_DEFINE_INTERFACE(FlView, fl_view, G_TYPE_OBJECT);

void fl_view_default_init(FlViewInterface* self) {}

G_MODULE_EXPORT FlEngine* fl_view_get_engine(FlView* self) {
  g_return_val_if_fail(FL_IS_VIEW(self), nullptr);

  FlViewInterface* iface = FL_VIEW_GET_IFACE(self);
  g_return_val_if_fail(iface->get_engine != nullptr, nullptr);

  return iface->get_engine(self);
}

G_MODULE_EXPORT
int64_t fl_view_get_id(FlView* self) {
  g_return_val_if_fail(FL_IS_VIEW(self), -1);

  FlViewInterface* iface = FL_VIEW_GET_IFACE(self);
  g_return_val_if_fail(iface->get_id != nullptr, -1);

  return iface->get_id(self);
}

G_MODULE_EXPORT void fl_view_set_background_color(FlView* self,
                                                  const GdkRGBA* color) {
  g_return_if_fail(FL_IS_VIEW(self));

  FlViewInterface* iface = FL_VIEW_GET_IFACE(self);
  g_return_if_fail(iface->set_background_color != nullptr);

  iface->set_background_color(self, color);
}
