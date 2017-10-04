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

AndroidPlatformSurfaceGL::~AndroidPlatformSurfaceGL() {
  if (texture_id_) {
    glDeleteTextures(1, &texture_id_);
  }
}

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
  if (new_frame_ready_) {
    if (!texture_id_) {
      glGenTextures(1, &texture_id_);
    }
    platform_view_->UpdateTexImage(Id(), texture_id_);
    new_frame_ready_ = false;
  }
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id_);
  GrGLTextureInfo textureInfo = {GL_TEXTURE_EXTERNAL_OES, texture_id_};
  GrBackendTexture backendTexture(width, height, kRGBA_8888_GrPixelConfig,
                                  textureInfo);
  return SkImage::MakeFromTexture(grContext, backendTexture,
                                  kTopLeft_GrSurfaceOrigin,
                                  SkAlphaType::kPremul_SkAlphaType, nullptr);
}

}  // namespace shell
