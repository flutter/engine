// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_PUBLIC_FLUTTER_PLUGIN_REGISTRAR_GLFW_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_PUBLIC_FLUTTER_PLUGIN_REGISTRAR_GLFW_H_

// This file contains additional plugin registrar-based functions available for
// the GLFW shell.

#include "flutter_glfw.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Returns the window associated with this registrar's engine instance.
FLUTTER_EXPORT FlutterDesktopWindowRef
FlutterDesktopRegistrarGetWindow(FlutterDesktopPluginRegistrarRef registrar);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_GLFW_PUBLIC_FLUTTER_PLUGIN_REGISTRAR_GLFW_H_
