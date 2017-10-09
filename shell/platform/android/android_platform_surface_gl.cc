// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_platform_surface_gl.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

#include "flutter/common/threads.h"
#include "flutter/shell/platform/android/platform_view_android_jni.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrTexture.h"

namespace shell {

AndroidPlatformSurfaceGL::AndroidPlatformSurfaceGL(
    const fml::jni::JavaObjectWeakGlobalRef& surface_texture)
    : surface_texture_(surface_texture) {}

AndroidPlatformSurfaceGL::~AndroidPlatformSurfaceGL() = default;

void AndroidPlatformSurfaceGL::OnGrContextCreated() {
  ASSERT_IS_GPU_THREAD;
  state_ = AttachmentState::uninitialized;
}

void AndroidPlatformSurfaceGL::MarkNewFrameAvailable() {
  ASSERT_IS_GPU_THREAD;
  new_frame_ready_ = true;
}

sk_sp<SkImage> AndroidPlatformSurfaceGL::MakeSkImage(int width,
                                                     int height,
                                                     GrContext* grContext) {
  ASSERT_IS_GPU_THREAD;
  if (state_ == AttachmentState::detached) {
    return nullptr;
  }
  if (state_ == AttachmentState::uninitialized) {
    glGenTextures(1, &texture_id_);
    Attach(static_cast<jint>(texture_id_));
    state_ = AttachmentState::attached;
  }
  if (new_frame_ready_) {
    Update();
    new_frame_ready_ = false;
  }
  GrGLTextureInfo textureInfo = {GL_TEXTURE_EXTERNAL_OES, texture_id_};
  GrBackendTexture backendTexture(width, height, kRGBA_8888_GrPixelConfig,
                                  textureInfo);
  return SkImage::MakeFromTexture(grContext, backendTexture,
                                  kTopLeft_GrSurfaceOrigin,
                                  SkAlphaType::kPremul_SkAlphaType, nullptr);
}

void AndroidPlatformSurfaceGL::OnGrContextDestroyed() {
  ASSERT_IS_GPU_THREAD;
  if (state_ == AttachmentState::attached) {
    Detach();
  }
  state_ = AttachmentState::detached;
}

void AndroidPlatformSurfaceGL::Attach(jint texture_id) {
  JNIEnv* env = fml::jni::AttachCurrentThread();
  fml::jni::ScopedJavaLocalRef<jobject> texture = surface_texture_.get(env);
  if (!texture.is_null()) {
    SurfaceTextureAttachToGLContext(env, texture.obj(), texture_id);
  }
}

void AndroidPlatformSurfaceGL::Update() {
  JNIEnv* env = fml::jni::AttachCurrentThread();
  fml::jni::ScopedJavaLocalRef<jobject> texture = surface_texture_.get(env);
  if (!texture.is_null()) {
    SurfaceTextureUpdateTexImage(env, texture.obj());
  }
}

void AndroidPlatformSurfaceGL::Detach() {
  JNIEnv* env = fml::jni::AttachCurrentThread();
  fml::jni::ScopedJavaLocalRef<jobject> texture = surface_texture_.get(env);
  if (!texture.is_null()) {
    SurfaceTextureDetachFromGLContext(env, texture.obj());
  }
}

}  // namespace shell
