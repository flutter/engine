// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_platform_view_factory.h"

#include <gmodule.h>

// Added here to stop the compiler from optimizing this function away.
G_MODULE_EXPORT GType fl_platform_view_factory_get_type();

G_DEFINE_INTERFACE(FlPlatformViewFactory,
                   fl_platform_view_factory,
                   G_TYPE_OBJECT)

static void fl_platform_view_factory_default_init(
    FlPlatformViewFactoryInterface* iface) {}

G_MODULE_EXPORT FlPlatformView* fl_platform_view_factory_create_platform_view(
    FlPlatformViewFactory* self,
    int64_t view_identifier,
    FlValue* args) {
  g_return_val_if_fail(FL_IS_PLATFORM_VIEW_FACTORY(self), nullptr);

  return FL_PLATFORM_VIEW_FACTORY_GET_IFACE(self)->create_platform_view(
      self, view_identifier, args);
}

G_MODULE_EXPORT FlMessageCodec*
fl_platform_view_factory_get_create_arguments_codec(
    FlPlatformViewFactory* self) {
  g_return_val_if_fail(FL_IS_PLATFORM_VIEW_FACTORY(self), nullptr);

  if (!FL_PLATFORM_VIEW_FACTORY_GET_IFACE(self)->get_create_arguments_codec)
    return nullptr;

  return FL_PLATFORM_VIEW_FACTORY_GET_IFACE(self)->get_create_arguments_codec(
      self);
}
