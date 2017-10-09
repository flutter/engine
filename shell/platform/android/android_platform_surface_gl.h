// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_SURFACE_GL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_SURFACE_GL_H_

#include "flutter/flow/platform_surface.h"
#include "flutter/fml/platform/android/jni_weak_ref.h"

namespace shell {

class AndroidPlatformSurfaceGL : public flow::PlatformSurface {
 public:
  AndroidPlatformSurfaceGL(
      const fml::jni::JavaObjectWeakGlobalRef& surface_texture);

  ~AndroidPlatformSurfaceGL() override;

  virtual sk_sp<SkImage> MakeSkImage(int width,
                                     int height,
                                     GrContext* grContext) override;

  virtual void OnGrContextCreated() override;

  virtual void OnGrContextDestroyed() override;

  // Called on GPU thread.
  void MarkNewFrameAvailable();

 private:
  void Attach(jint texName);

  void Update();

  void Detach();

  enum class AttachmentState { uninitialized, attached, detached };

  fml::jni::JavaObjectWeakGlobalRef surface_texture_;

  AttachmentState state_ = AttachmentState::uninitialized;

  bool new_frame_ready_ = false;

  uint32_t texture_id_ = 0;

  FXL_DISALLOW_COPY_AND_ASSIGN(AndroidPlatformSurfaceGL);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_SURFACE_GL_H_
