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

typedef struct {
  // RGBA pixel buffer.
  const uint8_t* buffer;
  // height and width.
  size_t width;
  size_t height;
} PixelBuffer;

// The pixel buffer copy callback definition is provided to
// the flutter engine to copy the texture.
typedef const PixelBuffer* (*FlutterTexutreCallback)(size_t width,
                                                     size_t height,
                                                     void* user_data);

// Register an new texture to the flutter engine and return the texture id,
// The engine will use the | texture_callback |
// function to copy the pixel buffer from the plugin caller.
FLUTTER_EXPORT int64_t FlutterDesktopRegisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    FlutterTexutreCallback texture_callback,
    void* user_data);

// Unregister an existing texture from the flutter engine for a |texture_id|.
FLUTTER_EXPORT bool FlutterDesktopUnregisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id);

// Mark that a new texture frame is available for a given |texture_id|.
FLUTTER_EXPORT bool FlutterDesktopMarkExternalTextureFrameAvailable(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_PUBLIC_FLUTTER_TEXTURE_REGISTRAR_H_
