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
#include "flutter/shell/platform/android/platform_view_android.h"
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
  ASSERT_IS_GPU_THREAD;
}

void AndroidPlatformSurfaceGL::Attach() {
  FXL_LOG(INFO) << "AndroidPlatformSurfaceGL::Attach";
  ASSERT_IS_GPU_THREAD;
  is_detached_ = false;
}

void AndroidPlatformSurfaceGL::MarkNewFrameAvailable() {
  ASSERT_IS_GPU_THREAD;
  new_frame_ready_ = true;
}

sk_sp<SkImage> AndroidPlatformSurfaceGL::MakeSkImage(int width,
                                                     int height,
                                                     GrContext* grContext) {
  ASSERT_IS_GPU_THREAD;
  if (is_detached_) {
    return nullptr;
  }
  if (!texture_id_) {
    glGenTextures(1, &texture_id_);
    platform_view_->AttachTexImage(Id(), texture_id_);
  }
  if (new_frame_ready_) {
    platform_view_->UpdateTexImage(Id());
    new_frame_ready_ = false;
  }
  GrGLTextureInfo textureInfo = {GL_TEXTURE_EXTERNAL_OES, texture_id_};
  GrBackendTexture backendTexture(width, height, kRGBA_8888_GrPixelConfig,
                                  textureInfo);
  return SkImage::MakeFromTexture(grContext, backendTexture,
                                  kTopLeft_GrSurfaceOrigin,
                                  SkAlphaType::kPremul_SkAlphaType, nullptr);
}

void AndroidPlatformSurfaceGL::Detach() {
  FXL_LOG(INFO) << "AndroidPlatformSurfaceGL::Detach";
  ASSERT_IS_GPU_THREAD;
  if (texture_id_) {
    platform_view_->DetachTexImage(Id());
    texture_id_ = 0;
  }
  is_detached_ = true;
}

}  // namespace shell
