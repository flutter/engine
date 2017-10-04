// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_platform_surface_gl.h"
#include "flutter/common/threads.h"
#include "flutter/lib/ui/painting/resource_context.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/vsync_waiter_ios.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrTexture.h"
#include "third_party/skia/include/gpu/GrTypes.h"

namespace shell {

IOSPlatformSurfaceGL::~IOSPlatformSurfaceGL() = default;

IOSPlatformSurfaceGL::IOSPlatformSurfaceGL(NSObject<FlutterPlatformSurface>* surface)
    : surface_(surface) {
  FXL_DCHECK(surface_);
}

sk_sp<SkImage> IOSPlatformSurfaceGL::MakeSkImage(int width, int height, GrContext* grContext) {
  ASSERT_IS_GPU_THREAD;
  if (!cacheRef_) {
    CVOpenGLESTextureCacheRef cache;
    CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL,
                                                [EAGLContext currentContext], NULL, &cache);
    if (err == noErr) {
      cacheRef_.Reset(cache);
    } else {
      FXL_LOG(WARNING) << "Failed to create GLES texture cache: " << err;
      return nullptr;
    }
  }
  fml::CFRef<CVPixelBufferRef> bufferRef;
  bufferRef.Reset([surface_ copyPixelBuffer]);
  if (bufferRef != nullptr) {
    CVOpenGLESTextureRef texture;
    CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(
        kCFAllocatorDefault, cacheRef_, bufferRef, nullptr, GL_TEXTURE_2D, GL_RGBA,
        static_cast<int>(CVPixelBufferGetWidth(bufferRef)),
        static_cast<int>(CVPixelBufferGetHeight(bufferRef)), GL_BGRA, GL_UNSIGNED_BYTE, 0,
        &texture);
    textureRef_.Reset(texture);
    if (err != noErr) {
      FXL_LOG(WARNING) << "Could not create texture from pixel buffer: " << err;
      return nullptr;
    }
  }
  if (!textureRef_) {
    return nullptr;
  }
  GrGLTextureInfo textureInfo = {CVOpenGLESTextureGetTarget(textureRef_),
                                 CVOpenGLESTextureGetName(textureRef_)};
  GrBackendTexture backendTexture(width, height, kRGBA_8888_GrPixelConfig, textureInfo);
  return SkImage::MakeFromTexture(grContext, backendTexture, kTopLeft_GrSurfaceOrigin,
                                  SkAlphaType::kPremul_SkAlphaType, nullptr);
}

}  // namespace shell
