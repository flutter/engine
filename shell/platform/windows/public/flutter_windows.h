// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_PUBLIC_FLUTTER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_PUBLIC_FLUTTER_H_

#include <stddef.h>
#include <stdint.h>

#include "flutter_export.h"
#include "flutter_messenger.h"
#include "flutter_plugin_registrar.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Opaque reference to a Flutter window controller.
typedef struct FlutterDesktopWindowControllerState*
    FlutterDesktopWindowControllerRef;

// Opaque reference to a Flutter window.
typedef struct FlutterDesktopWindow* FlutterDesktopWindowRef;

// Opaque reference to a Flutter engine instance.
typedef struct FlutterDesktopEngineState* FlutterDesktopEngineRef;

// TODO: remove once the embedder project has swiched to native windows
FLUTTER_EXPORT bool FlutterDesktopInit();

// TODO: remove once the embedder project has swiched to native windows
FLUTTER_EXPORT void FlutterDesktopTerminate();

// Creates a Window running a Flutter Application.
//
// FlutterDesktopInit() must be called prior to this function.
//
// The |assets_path| is the path to the flutter_assets folder for the Flutter
// application to be run. |icu_data_path| is the path to the icudtl.dat file
// for the version of Flutter you are using.
//
// The |arguments| are passed to the Flutter engine. See:
// https://github.com/flutter/engine/blob/master/shell/common/switches.h for
// for details. Not all arguments will apply to desktop.
//
// Returns a null pointer in the event of an error. Otherwise, the pointer is
// valid until FlutterDesktopRunWindowLoop has been called and returned.
// Note that calling FlutterDesktopCreateWindow without later calling
// FlutterDesktopRunWindowLoop on the returned reference is a memory leak.
FLUTTER_EXPORT FlutterDesktopWindowControllerRef
FlutterDesktopCreateWindow(int initial_width,
                           int initial_height,
                           const char* title,
                           const char* assets_path,
                           const char* icu_data_path,
                           const char** arguments,
                           size_t argument_count);

FLUTTER_EXPORT FlutterDesktopWindowControllerRef
FlutterDesktopCreateView(int initial_width,
                         int initial_height,
                         const char* assets_path,
                         const char* icu_data_path,
                         const char** arguments,
                         size_t argument_count);

// Shuts down the engine instance associated with |controller|, and cleans up
// associated state.
//
// |controller| is no longer valid after this call.
FLUTTER_EXPORT void FlutterDesktopDestroyWindow(
    FlutterDesktopWindowControllerRef controller);

// Returns the window handle for the window associated with
// FlutterDesktopWindowControllerRef.
//
// Its lifetime is the same as the |controller|'s.
FLUTTER_EXPORT FlutterDesktopWindowRef
FlutterDesktopGetWindow(FlutterDesktopWindowControllerRef controller);

// Returns the plugin registrar handle for the plugin with the given name.
//
// The name must be unique across the application.
FLUTTER_EXPORT FlutterDesktopPluginRegistrarRef
FlutterDesktopGetPluginRegistrar(FlutterDesktopWindowControllerRef controller,
                                 const char* plugin_name);
// TODO
FLUTTER_EXPORT long FlutterDesktopGetHWNDFromView(
    FlutterDesktopWindowRef flutter_window);

// TODO
FLUTTER_EXPORT void FlutterDesktopProcessMessages();

// Runs an instance of a headless Flutter engine.
//
// The |assets_path| is the path to the flutter_assets folder for the Flutter
// application to be run. |icu_data_path| is the path to the icudtl.dat file
// for the version of Flutter you are using.
//
// The |arguments| are passed to the Flutter engine. See:
// https://github.com/flutter/engine/blob/master/shell/common/switches.h for
// for details. Not all arguments will apply to desktop.
//
// Returns a null pointer in the event of an error.
FLUTTER_EXPORT FlutterDesktopEngineRef
FlutterDesktopRunEngine(const char* assets_path,
                        const char* icu_data_path,
                        const char** arguments,
                        size_t argument_count);

// Shuts down the given engine instance. Returns true if the shutdown was
// successful. |engine_ref| is no longer valid after this call.
FLUTTER_EXPORT bool FlutterDesktopShutDownEngine(
    FlutterDesktopEngineRef engine_ref);

// TODO: remove once embedder project has switched to native windows
// imlpementation
FLUTTER_EXPORT FlutterDesktopWindowRef
FlutterDesktopRegistrarGetWindow(FlutterDesktopPluginRegistrarRef registrar);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PUBLIC_FLUTTER_WINDOWS_H_
