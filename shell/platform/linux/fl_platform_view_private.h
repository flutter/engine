// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEW_PRIVATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEW_PRIVATE_H_

#include "flutter/shell/platform/linux/public/flutter_linux/fl_platform_view.h"

G_BEGIN_DECLS

/**
 * fl_platform_view_set_direction:
 * @platform_view: an #FlPlatformView.
 * @direction: new #GtkTextDirection for platform view.
 *
 * Set text direction.
 */
void fl_platform_view_set_direction(FlPlatformView* platform_view,
                                    GtkTextDirection direction);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_PLATFORM_VIEW_PRIVATE_H_
