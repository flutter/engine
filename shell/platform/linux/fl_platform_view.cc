// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_platform_view_factory.h"

#include "flutter/shell/platform/linux/fl_platform_view_private.h"

#include <gmodule.h>

// Added here to stop the compiler from optimizing this function away.
G_MODULE_EXPORT GType fl_platform_view_get_type();

typedef struct {
  GtkTextDirection direction;
} FlPlatformViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(FlPlatformView, fl_platform_view, G_TYPE_OBJECT)

static void fl_platform_view_class_init(FlPlatformViewClass* klass) {}

static void fl_platform_view_init(FlPlatformView* self) {}

G_MODULE_EXPORT GtkWidget* fl_platform_view_get_view(FlPlatformView* self) {
  g_return_val_if_fail(FL_IS_PLATFORM_VIEW(self), nullptr);

  GtkWidget* widget = FL_PLATFORM_VIEW_GET_CLASS(self)->get_view(self);
  if (!widget || !GTK_IS_WIDGET(widget)) {
    g_critical("fl_platform_view::get_view should return GtkWidget");
    return nullptr;
  }

  FlPlatformViewPrivate* priv = static_cast<FlPlatformViewPrivate*>(
      fl_platform_view_get_instance_private(self));
  gtk_widget_set_direction(widget, priv->direction);

  return widget;
}

void fl_platform_view_set_direction(FlPlatformView* self,
                                    GtkTextDirection direction) {
  FlPlatformViewPrivate* priv = static_cast<FlPlatformViewPrivate*>(
      fl_platform_view_get_instance_private(self));
  priv->direction = direction;
}
