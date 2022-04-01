// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_platform_view.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_platform_view_factory.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlMockPlatformView,
                     fl_mock_platform_view,
                     FL,
                     MOCK_PLATFORM_VIEW,
                     FlPlatformView)

FlMockPlatformView* fl_mock_platform_view_new(
    GtkWidget* (*get_view)(FlPlatformView* self));

int64_t fl_mock_platform_view_get_view_identifier(
    FlMockPlatformView* platform_view);

G_DECLARE_FINAL_TYPE(FlMockViewFactory,
                     fl_mock_view_factory,
                     FL,
                     MOCK_VIEW_FACTORY,
                     GObject)

FlMockViewFactory* fl_mock_view_factory_new(
    FlPlatformView* (*create_platform_view)(FlPlatformViewFactory* self,
                                            int64_t view_identifier,
                                            FlValue* args),
    FlMessageCodec* (*get_create_arguments_codec)(FlPlatformViewFactory* self));

G_END_DECLS
