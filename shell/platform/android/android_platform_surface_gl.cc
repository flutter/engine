// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_platform_surface_gl.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <jni.h>
#include "flutter/common/threads.h"
#include "flutter/lib/ui/painting/resource_context.h"
#include "flutter/shell/platform/android/platform_view_android_jni.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrTexture.h"
#include "third_party/skia/include/gpu/GrTypes.h"

namespace shell {

AndroidPlatformSurfaceGL::~AndroidPlatformSurfaceGL() = default;

AndroidPlatformSurfaceGL::AndroidPlatformSurfaceGL(
    std::shared_ptr<PlatformViewAndroid> platformView)
    : platform_view_(platformView) {
  ASSERT_IS_PLATFORM_THREAD;
}

class CleanupContext {
 public:
  int surface_id;
  std::shared_ptr<PlatformViewAndroid> platform_view;
};

void AndroidPlatformSurfaceGL::MarkNewFrameAvailable() {
  ASSERT_IS_PLATFORM_THREAD;
  blink::Threads::Gpu()->PostTask([this]() { new_frame_ready_ = true; });
}

sk_sp<SkImage> AndroidPlatformSurfaceGL::MakeSkImage(int width,
                                                     int height,
                                                     GrContext* grContext) {
  ASSERT_IS_GPU_THREAD;
  GrGLuint texID;
  if (new_frame_ready_) {
    glGenTextures(1, &texID);
    platform_view_->UpdateTexImage(Id(), texID);
    new_frame_ready_ = false;
  } else {
    return nullptr;
  }
  GrGLTextureInfo textureInfo = {GL_TEXTURE_EXTERNAL_OES, texID};
  GrBackendTexture backendTexture(width, height, kRGBA_8888_GrPixelConfig,
                                  textureInfo);
  CleanupContext* cleanupContext = new CleanupContext{Id(), platform_view_};
  sk_sp<SkImage> sk_image = SkImage::MakeFromTexture(
      grContext, backendTexture, kTopLeft_GrSurfaceOrigin,
      SkAlphaType::kPremul_SkAlphaType, nullptr,
      [](void* releaseContext) {
        CleanupContext* ctx = (CleanupContext*)releaseContext;
        ctx->platform_view->DetachTexImage(ctx->surface_id);
        delete ctx;
      },
      cleanupContext);
  return sk_image;
}

}  // namespace shell
