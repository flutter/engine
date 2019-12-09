// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_PUBLIC_FLUTTER_TEXTURE_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_PUBLIC_FLUTTER_TEXTURE_REGISTRAR_H_

#include <stddef.h>
#include <stdint.h>

#include "flutter_export.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Opaque reference to a texture registrar.
typedef struct FlutterDesktopTextureRegistrar*
    FlutterDesktopTextureRegistrarRef;

// Constructs a pixel buffer object for the plugin side, providing
// height/width and buffer pointers.
typedef struct {
  // Bitmap buffer pointer, currently only supports RGBA.
  const uint8_t* buffer;
  // Width of the pixel buffer.
  size_t width;
  // Height of the pixel buffer.
  size_t height;
} PixelBuffer;

// The pixel buffer copy callback definition is provided to
// the Flutter engine to copy the texture.
typedef const PixelBuffer* (*FlutterTextureCallback)(size_t width,
                                                     size_t height,
                                                     void* user_data);

// Registers a new texture with the Flutter engine and returns the texture ID,
// The engine will use the |texture_callback|
// function to copy the pixel buffer from the plugin caller.
FLUTTER_EXPORT int64_t FlutterDesktopRegisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    FlutterTextureCallback texture_callback,
    void* user_data);

// Unregisters an existing texture from the Flutter engine for a |texture_id|.
// Returns true on success, false on failure.
FLUTTER_EXPORT bool FlutterDesktopUnregisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id);

// Marks that a new texture frame is available for a given |texture_id|.
// Returns true on success, false on failure.
FLUTTER_EXPORT bool FlutterDesktopMarkExternalTextureFrameAvailable(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_PUBLIC_FLUTTER_TEXTURE_REGISTRAR_H_
