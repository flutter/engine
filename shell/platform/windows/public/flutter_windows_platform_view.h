// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_PUBLIC_FLUTTER_WINDOWS_PLATFORM_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_PUBLIC_FLUTTER_WINDOWS_PLATFORM_VIEW_H_

#include "flutter_windows.h"

struct IDCompositionDevice;
struct IDCompositionVisual;

#if defined(__cplusplus)
extern "C" {
#endif

/// Whether Platform Views are supported on the current OS version.
/// The rest of the Platform View methods will fail if unsupported.
///
/// Platform Views require Windows 8+ and DX12.
FLUTTER_EXPORT bool FlutterDesktopPlatformViewsSupported();

/// Returns the IDCompositionDevice used to create visuals internally.
/// This can be used to create visuals that automatically get committed in sync with
/// the Flutter compositions.
FLUTTER_EXPORT struct IDCompositionDevice* FlutterDesktopPlatformViewsCompositionDevice(FlutterDesktopViewRef view_ref);

typedef struct {
  /// horizontal scale factor
  double scaleX;
  /// horizontal skew factor
  double skewX;
  /// horizontal translation
  double transX;
  /// vertical skew factor
  double skewY;
  /// vertical scale factor
  double scaleY;
  /// vertical translation
  double transY;
  /// input x-axis perspective factor
  double pers0;
  /// input y-axis perspective factor
  double pers1;
  /// perspective scale factor
  double pers2;
} FlutterDesktopTransformation;

/// A structure to represent the width and height.
typedef struct {
  double width;
  double height;
} FlutterDesktopSize;

typedef struct {
    // Current total transform of the platform view.
    // Coordinate space is physical pixels.
    FlutterDesktopTransformation transform;
    // Whether the above transform represents a simple translation.
    // The platform view is not scaled or skewed.
    bool is_simple_translate;
    // Current requested dimensions of the platform view in physical pixels.
    // Embedder should resize or clip contents to match this size.
    // This is the size before the transformation matrix above takes effect.
    FlutterDesktopSize size;
} FlutterDesktopPlatformViewUpdate;

// TODO: Use an invisible topmost window with NCHITTEST for platform view clipping.
// It'll have to call into Flutter synchronously to hit test.

typedef void (*FlutterDesktopPlatformViewUpdateVisibleCallback)(void* /* user_data */, const FlutterDesktopPlatformViewUpdate* /* update */);
typedef void (*FlutterDesktopPlatformViewUpdateHiddenCallback)(void* /* user_data */);

typedef struct {
    size_t struct_size;
    // DirectComposition Visual that is presented.
    // Must not be null.
    struct IDCompositionVisual* visual;
    // TODO: NYI
    // Window of the visual that is positioned over the view (but transparent).
    // This is positioned so it receives mouse input from the application.
    // Can be null.
    // The size of this window will be updated to match the size presented by the
    // application.
    // The window is reparented under Flutter's window.
    // If the window cannot receive input (due to a non-trivial transform) the
    // window is disabled with EnableWindow so it cannot get mouse or keyboard
    // input.
    HWND window;
    void* user_data;

    // Called when a platform view is visible within a frame.
    FlutterDesktopPlatformViewUpdateVisibleCallback update_visible_callback;

    // Called when a platform view is no longer visible within a frame.
    FlutterDesktopPlatformViewUpdateHiddenCallback update_hidden_callback;
} FlutterDesktopPlatformView;

FLUTTER_EXPORT int64_t FlutterDesktopRegisterPlatformView(FlutterDesktopViewRef view_ref, const FlutterDesktopPlatformView *view);

FLUTTER_EXPORT void FlutterDesktopUnregisterPlatformView(FlutterDesktopViewRef view_ref, int64_t view_id);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PUBLIC_FLUTTER_WINDOWS_PLATFORM_VIEW_H_
