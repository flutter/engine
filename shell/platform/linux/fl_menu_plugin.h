// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_MENU_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_MENU_PLUGIN_H_

#include <gdk/gdk.h>

#include "flutter/shell/platform/linux/public/flutter_linux/fl_binary_messenger.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

// API to control a native menu bar.
G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlMenuPlugin, fl_menu_plugin, FL, MENU_PLUGIN, GObject)

FlMenuPlugin* fl_menu_plugin_new(FlBinaryMessenger* messenger, FlView* view);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_MENU_PLUGIN_H_
