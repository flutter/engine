// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_CPP_PUBLIC_FLUTTER_GLFW_H_
#define FLUTTER_SHELL_PLATFORM_CPP_PUBLIC_FLUTTER_GLFW_H_

#include <stddef.h>
#include <stdint.h>

#include "flutter_export.h"
#include "flutter_messenger.h"
#include "flutter_plugin_registrar.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Opaque reference to a Flutter window.
typedef struct FlutterEmbedderState* FlutterWindowRef;
// XXX Change FlutterEmbedder* everywhere.

// Opaque reference to a Flutter engine instance.
typedef struct FlutterEngineState* FlutterEngineRef;

// Sets up the embedder's graphic context. Must be called before any other
// methods.
//
// Note: Internally, this library uses GLFW, which does not support multiple
// copies within the same process. Internally this calls glfwInit, which will
// fail if you have called glfwInit elsewhere in the process.
FLUTTER_EXPORT bool FlutterEmbedderInit();

// Tears down embedder state. Must be called before the process terminates.
FLUTTER_EXPORT void FlutterEmbedderTerminate();

// Creates a Window running a Flutter Application.
//
// FlutterEmbedderInit() must be called prior to this function.
//
// The |assets_path| is the path to the flutter_assets folder for the Flutter
// application to be run. |icu_data_path| is the path to the icudtl.dat file
// for the version of Flutter you are using.
//
// The |arguments| are passed to the Flutter engine. See:
// https://github.com/flutter/engine/blob/master/shell/common/switches.h for
// for details. Not all arguments will apply to embedding mode.
//
// Returns a null pointer in the event of an error. Otherwise, the pointer is
// valid until FlutterEmbedderRunWindowLoop has been called and returned.
// Note that calling FlutterEmbedderCreateWindow without later calling
// FlutterEmbedderRunWindowLoop on the returned reference is a memory leak.
FLUTTER_EXPORT FlutterWindowRef
FlutterEmbedderCreateWindow(int initial_width,
                            int initial_height,
                            const char* assets_path,
                            const char* icu_data_path,
                            const char** arguments,
                            size_t argument_count);

// Runs an instance of a headless Flutter engine.
//
// The |assets_path| is the path to the flutter_assets folder for the Flutter
// application to be run. |icu_data_path| is the path to the icudtl.dat file
// for the version of Flutter you are using.
//
// The |arguments| are passed to the Flutter engine. See:
// https://github.com/flutter/engine/blob/master/shell/common/switches.h for
// for details. Not all arguments will apply to embedding mode.
//
// Returns a null pointer in the event of an error.
FLUTTER_EXPORT FlutterEngineRef
FlutterEmbedderRunEngine(const char* assets_path,
                         const char* icu_data_path,
                         const char** arguments,
                         size_t argument_count);

// Shuts down the given engine instance. Returns true if the shutdown was
// successful. |engine_ref| is no longer valid after this call.
FLUTTER_EXPORT bool FlutterEmbedderShutDownEngine(FlutterEngineRef engine_ref);

// Enables or disables hover tracking.
//
// If hover is enabled, mouse movement will send hover events to the Flutter
// engine, rather than only tracking the mouse while the button is pressed.
// Defaults to off.
FLUTTER_EXPORT void FlutterEmbedderSetHoverEnabled(
    FlutterWindowRef flutter_window,
    bool enabled);

// Loops on Flutter window events until the window is closed.
//
// Once this function returns, FlutterWindowRef is no longer valid, and must
// not be used again.
FLUTTER_EXPORT void FlutterEmbedderRunWindowLoop(
    FlutterWindowRef flutter_window);

// Returns the plugin registrar handle for the plugin with the given name.
//
// The name must be unique across the application.
FLUTTER_EXPORT FlutterEmbedderPluginRegistrarRef
FlutterEmbedderGetPluginRegistrar(FlutterWindowRef flutter_window,
                                  const char* plugin_name);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_CPP_PUBLIC_FLUTTER_GLFW_H_
