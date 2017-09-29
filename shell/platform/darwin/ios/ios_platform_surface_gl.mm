// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/common/threads.h"
#include "flutter/lib/ui/painting/resource_context.h"
#include "flutter/shell/platform/darwin/ios/ios_platform_surface_gl.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/vsync_waiter_ios.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrTexture.h"
#include "third_party/skia/include/gpu/GrTypes.h"

namespace shell {

IOSPlatformSurfaceGL::~IOSPlatformSurfaceGL() {
  // TODO(sigurdm, mravn): dispose cache
}

IOSPlatformSurfaceGL::IOSPlatformSurfaceGL(NSObject<FlutterPlatformSurface>* surface): surface_(surface) {
  FXL_DCHECK(surface_);
}

sk_sp<SkImage> IOSPlatformSurfaceGL::MakeSkImage(int width, int height, GrContext *grContext) {
  ASSERT_IS_GPU_THREAD;
  CVPixelBufferRef buffer;
  fxl::AutoResetWaitableEvent latch;
  blink::Threads::IO()->PostTask([this, &latch, &buffer]() {
    if (cache_ == nullptr) {
      CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, [EAGLContext currentContext], NULL, &cache_);
      if (err != noErr) {
        FXL_LOG(WARNING) << "Failed to create GLES texture cache: " << err;
      }
    }
    buffer = [surface_ getPixelBuffer];
    latch.Signal();
  });
  latch.Wait();
  if (buffer != nullptr) {
    if (texture_ != nullptr) CFRelease(texture_);
    CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(
      kCFAllocatorDefault, cache_, buffer, nullptr,
      GL_TEXTURE_2D,
      GL_RGBA,
      (int) CVPixelBufferGetWidth(buffer),
      (int) CVPixelBufferGetHeight(buffer),
      GL_BGRA,
      GL_UNSIGNED_BYTE,
      0,
      &texture_);
      CFRelease(buffer);
    if (err != noErr) {
      FXL_LOG(WARNING) << "Could not create texture from pixel buffer: " << err;
      return nullptr;
    }
  }
  if (texture_ == nullptr) {
    return nullptr;
  }
  GrGLTextureInfo textureInfo = {CVOpenGLESTextureGetTarget(texture_), CVOpenGLESTextureGetName(texture_)};
  GrBackendTexture backendTexture(width, height, kRGBA_8888_GrPixelConfig, textureInfo);
  sk_sp<SkImage> sk_image = SkImage::MakeFromTexture(
     grContext,
     backendTexture,
     kBottomLeft_GrSurfaceOrigin,
     SkAlphaType::kPremul_SkAlphaType,
     nullptr
  );
  CVOpenGLESTextureCacheFlush(cache_, 0);
  return sk_image;
}

} // namespace shell
