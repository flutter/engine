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

struct FlutterDesktopTextureRegistrar;
// Opaque reference to a texture registrar.
typedef struct FlutterDesktopTextureRegistrar*
    FlutterDesktopTextureRegistrarRef;

// An image buffer object.
typedef struct {
  // The pixel data buffer.
  const uint8_t* buffer;
  // Width of the pixel buffer.
  size_t width;
  // Height of the pixel buffer.
  size_t height;
} FlutterDesktopPixelBuffer;

// The pixel buffer copy callback definition provided to
// the Flutter engine to copy the texture.
typedef const FlutterDesktopPixelBuffer* (*FlutterDesktopTextureCallback)(
    size_t width,
    size_t height,
    void* user_data);

// Registers a new texture with the Flutter engine and returns the texture ID,
// The engine will use the |texture_callback|
// function to copy the pixel buffer from the caller.
FLUTTER_EXPORT int64_t FlutterDesktopTextureRegistrarRegisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    FlutterDesktopTextureCallback texture_callback,
    void* user_data);

// Unregisters an existing texture from the Flutter engine for a |texture_id|.
// Returns true on success or false if the specified texture doesn't exist.
FLUTTER_EXPORT bool FlutterDesktopTextureRegistrarUnregisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id);

// Marks that a new texture frame is available for a given |texture_id|.
// Returns true on success or false if the specified texture doesn't exist.
FLUTTER_EXPORT bool
FlutterDesktopTextureRegistrarMarkExternalTextureFrameAvailable(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_PUBLIC_FLUTTER_TEXTURE_REGISTRAR_H_
