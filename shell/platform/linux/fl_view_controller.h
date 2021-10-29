// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_VIEW_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_VIEW_CONTROLLER_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include "fl_dart_project.h"
#include "fl_engine.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlViewController, fl_view_controller, FL, VIEW_CONTROLLER, GObject)

FlViewController* fl_view_controller_new(FlDartProject* project);

/**
 * fl_view_get_engine:
 * @view: an #FlView.
 *
 * Gets the engine being rendered in the view.
 *
 * Returns: an #FlEngine.
 */
FlEngine* fl_view_controller_get_engine(FlViewController* view);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_VIEW_CONTROLLER_H_
