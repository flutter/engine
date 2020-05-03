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

#include "Windows.h"
#include "windows.ui.composition.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Opaque reference to a Flutter window controller.
typedef struct FlutterDesktopViewControllerState*
    FlutterDesktopViewControllerRef;

// Opaque reference to a Flutter window controller.
typedef struct FlutterDesktopViewControllerState*
    V2FlutterDesktopViewControllerRef;

// Opaque reference to a Flutter window.
typedef struct FlutterDesktopView* FlutterDesktopViewRef;

// Opaque reference to a Flutter engine instance.
typedef struct FlutterDesktopEngineState* FlutterDesktopEngineRef;

// Opaque reference to host specific state
typedef void* HostEnvironmentState;

// Properties for configuring a Flutter engine instance.
typedef struct {
  // The path to the flutter_assets folder for the application to be run.
  // This can either be an absolute path or a path relative to the directory
  // containing the executable.
  const wchar_t* assets_path;

  // The path to the icudtl.dat file for the version of Flutter you are using.
  // This can either be an absolute path or a path relative to the directory
  // containing the executable.
  const wchar_t* icu_data_path;

  // The switches to pass to the Flutter engine.
  //
  // See: https://github.com/flutter/engine/blob/master/shell/common/switches.h
  // for details. Not all arguments will apply to desktop.
  const char** switches;

  // The number of elements in |switches|.
  size_t switches_count;
} FlutterDesktopEngineProperties;

// Creates a View with the given dimensions running a Flutter Application.
//
// This will set up and run an associated Flutter engine using the settings in
// |engine_properties|.
//
// View supplied must be integrated into host visual tree
//
// Returns a null pointer in the event of an error.
FLUTTER_EXPORT FlutterDesktopViewControllerRef
V2CreateViewControllerVisual(
    int width,
    int height,
    const FlutterDesktopEngineProperties& engine_properties,
    ABI::Windows::UI::Composition::IVisual* visual,
    HostEnvironmentState hoststate);

// Creates a View with the given dimensions running a Flutter Application.
//
// This will set up and run an associated Flutter engine using the settings in
// |engine_properties|.
//
// Returns a null pointer in the event of an error.
FLUTTER_EXPORT FlutterDesktopViewControllerRef
V2CreateViewControllerWindow(
    int width,
    int height,
    const FlutterDesktopEngineProperties& engine_properties,
    HWND windowrendertarget,
    HostEnvironmentState hostwindow
    );

// Return backing HWND for manipulation in host application.
FLUTTER_EXPORT
HostEnvironmentState V2FlutterDesktopGetHostState(FlutterDesktopViewRef view);

//TODO
FLUTTER_EXPORT void V2FlutterDesktopSendWindowMetrics(FlutterDesktopViewRef view,
                                                    size_t width,
                                                    size_t height,
                                                    double dpiScale);

FLUTTER_EXPORT void V2FlutterDesktopSendPointerMove(FlutterDesktopViewRef view,
                                                  double x,
                                                  double y);

FLUTTER_EXPORT void V2FlutterDesktopSendPointerDown(
    FlutterDesktopViewRef view,
    double x,
    double y,
    uint64_t btn);  // TODO: can / should this be a FlutterPointerMouseButtons

FLUTTER_EXPORT void V2FlutterDesktopSendPointerUp(FlutterDesktopViewRef view,
                                                double x,
                                                double y,
                                                uint64_t btn);  //TODO typedef per above

FLUTTER_EXPORT void V2FlutterDesktopSendPointerLeave(FlutterDesktopViewRef view);

// TODO
FLUTTER_EXPORT void V2FlutterDesktopSendScroll(FlutterDesktopViewRef view,
                                             double x,
                                             double y,
                                             double delta_x,
                                             double delta_y);

// TODO
FLUTTER_EXPORT void V2FlutterDesktopSendFontChange(FlutterDesktopViewRef view);

// TODO
FLUTTER_EXPORT void V2FlutterDesktopSendText(FlutterDesktopViewRef view,
                                           const char16_t* code_point,
                                           size_t size);

// TODO
FLUTTER_EXPORT void V2FlutterDesktopSendKey(FlutterDesktopViewRef view,
                                          int key,
                                          int scancode,
                                          int action,
                                          char32_t character);


// Creates a View with the given dimensions running a Flutter Application.
//
// This will set up and run an associated Flutter engine using the settings in
// |engine_properties|.
//
// Returns a null pointer in the event of an error.
//FLUTTER_EXPORT FlutterDesktopViewControllerRef
//FlutterDesktopCreateViewController(
//    int width,
//    int height,
//    const FlutterDesktopEngineProperties& engine_properties);

//// DEPRECATED. Will be removed soon; switch to the version above.
//FLUTTER_EXPORT FlutterDesktopViewControllerRef
//FlutterDesktopCreateViewControllerLegacy(int initial_width,
//                                         int initial_height,
//                                         const char* assets_path,
//                                         const char* icu_data_path,
//                                         const char** arguments,
//                                         size_t argument_count);

// Shuts down the engine instance associated with |controller|, and cleans up
// associated state.
//
// |controller| is no longer valid after this call.
FLUTTER_EXPORT void FlutterDesktopDestroyViewController(
    FlutterDesktopViewControllerRef controller);

// Returns the plugin registrar handle for the plugin with the given name.
//
// The name must be unique across the application.
FLUTTER_EXPORT FlutterDesktopPluginRegistrarRef
FlutterDesktopGetPluginRegistrar(FlutterDesktopViewControllerRef controller,
                                 const char* plugin_name);

// Returns the view managed by the given controller.
FLUTTER_EXPORT FlutterDesktopViewRef
FlutterDesktopGetView(FlutterDesktopViewControllerRef controller);

// Processes any pending events in the Flutter engine, and returns the
// number of nanoseconds until the next scheduled event (or  max, if none).
//
// This should be called on every run of the application-level runloop, and
// a wait for native events in the runloop should never be longer than the
// last return value from this function.
FLUTTER_EXPORT uint64_t
FlutterDesktopProcessMessages(FlutterDesktopViewControllerRef controller);

// Reopens stdout and stderr and resysncs the standard library output streams.
// Should be called if output is being directed somewhere in the runner process
// (e.g., after an AllocConsole call).
FLUTTER_EXPORT void FlutterDesktopResyncOutputStreams();

// Runs an instance of a headless Flutter engine.
//
// Returns a null pointer in the event of an error.
FLUTTER_EXPORT FlutterDesktopEngineRef FlutterDesktopRunEngine(
    const FlutterDesktopEngineProperties& engine_properties);

// Shuts down the given engine instance. Returns true if the shutdown was
// successful. |engine_ref| is no longer valid after this call.
FLUTTER_EXPORT bool FlutterDesktopShutDownEngine(
    FlutterDesktopEngineRef engine_ref);

// Returns the view associated with this registrar's engine instance
// This is a Windows-specific extension to flutter_plugin_registrar.h.
FLUTTER_EXPORT FlutterDesktopViewRef
FlutterDesktopRegistrarGetView(FlutterDesktopPluginRegistrarRef registrar);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PUBLIC_FLUTTER_WINDOWS_H_
