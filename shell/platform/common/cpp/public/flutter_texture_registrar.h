// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_PUBLIC_FLUTTER_TEXTURE_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_PUBLIC_FLUTTER_TEXTURE_REGISTRAR_H_

#include <stddef.h>
#include <stdint.h>
#include <memory>

#include "flutter_export.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Opaque reference to a texture registrar.
typedef struct FlutterDesktopTextureRegistrar* FlutterDesktopTextureRegistrarRef;

typedef std::shared_ptr<uint8_t> (*FlutterTexutreCallback)(size_t width,
                                                           size_t height,
                                                           void* user_data);

FLUTTER_EXPORT int64_t FlutterDesktopRegisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    FlutterTexutreCallback texture_callback,
    void* user_data);

FLUTTER_EXPORT bool FlutterDesktopUnregisterExternalTexture(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id);

// Mark that a new texture frame is available for a given texture identifier.
FLUTTER_EXPORT bool FlutterDesktopMarkExternalTextureFrameAvailable(
    FlutterDesktopTextureRegistrarRef texture_registrar,
    int64_t texture_id);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_PUBLIC_FLUTTER_TEXTURE_REGISTRAR_H_
