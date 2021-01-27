// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_VIEW_PRIVATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_VIEW_PRIVATE_H_

#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

#include "flutter/shell/platform/linux/fl_gl_area.h"
#include "flutter/shell/platform/linux/fl_platform_views_plugin.h"

FlPlatformViewsPlugin* fl_view_get_platform_views_plugin(FlView* view);

void fl_view_begin_frame(FlView* view);

void fl_view_add_gl_area(FlView* view,
                         GdkGLContext* context,
                         FlGLAreaTexture* texture);

void fl_view_add_widget(FlView* view,
                        GtkWidget* widget,
                        GdkRectangle* geometry);

void fl_view_end_frame(FlView* view);

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_VIEW_PRIVATE_H_
