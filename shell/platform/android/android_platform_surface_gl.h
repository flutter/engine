// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_SURFACE_GL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_SURFACE_GL_H_

#include <jni.h>
#include "flutter/flow/platform_surface.h"
#include "flutter/fml/platform/android/jni_util.h"
#include "flutter/fml/platform/android/jni_weak_ref.h"
#include "flutter/fml/platform/android/scoped_java_ref.h"
#include "flutter/shell/platform/android/platform_view_android.h"

namespace shell {

class AndroidPlatformSurfaceGL : public flow::PlatformSurface {
 public:
  AndroidPlatformSurfaceGL(std::shared_ptr<PlatformViewAndroid> platformView);

  ~AndroidPlatformSurfaceGL() override;

  // Called from GPU thread.
  virtual sk_sp<SkImage> MakeSkImage(int width,
                                     int height,
                                     GrContext* grContext) override;

  // Called on platform thread.
  void MarkNewFrameAvailable();

 private:
  std::shared_ptr<PlatformViewAndroid> platform_view_;
  bool new_frame_ready_ = false;
  FXL_DISALLOW_COPY_AND_ASSIGN(AndroidPlatformSurfaceGL);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_SURFACE_GL_H_
