// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_platform_views.h"

#include <EGL/egl.h>
#include <gmodule.h>

// Added here to stop the compiler from optimizing this function away.
G_MODULE_EXPORT GType fl_platform_view_get_type();

G_DEFINE_TYPE(FlPlatformView, fl_platform_view, G_TYPE_OBJECT)

static void fl_platform_view_class_init(FlPlatformViewClass* klass) {}

static void fl_platform_view_init(FlPlatformView* self) {}

G_MODULE_EXPORT GtkWidget* fl_platform_view_get_view(FlPlatformView* self) {
  g_return_val_if_fail(FL_IS_PLATFORM_VIEW(self), nullptr);

  GtkWidget* widget = FL_PLATFORM_VIEW_GET_CLASS(self)->get_view(self);
  if (widget && !GTK_IS_WIDGET(widget)) {
    g_critical("fl_platform_view::get_view should return GtkWidget");
  }
  return widget;
}

// Added here to stop the compiler from optimizing this function away.
G_MODULE_EXPORT GType fl_platform_view_factory_get_type();

G_DEFINE_INTERFACE(FlPlatformViewFactory,
                   fl_platform_view_factory,
                   G_TYPE_OBJECT)

static void fl_platform_view_factory_default_init(
    FlPlatformViewFactoryInterface* iface) {
  // nothing to do
}

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

  return FL_PLATFORM_VIEW_FACTORY_GET_IFACE(self)->get_create_arguments_codec(
      self);
}
