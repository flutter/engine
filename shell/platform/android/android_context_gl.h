// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_CONTEXT_GL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_CONTEXT_GL_H_

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/platform/android/android_context.h"
#include "flutter/shell/platform/android/android_environment_gl.h"
#include "flutter/shell/platform/android/android_native_window.h"
#include "third_party/skia/include/core/SkSize.h"

namespace flutter {

class AndroidContextGL : public fml::RefCountedThreadSafe<AndroidContextGL>,
                         public AndroidContext {
 public:
  AndroidContextGL(AndroidRenderingAPI rendering_api,
                   fml::RefPtr<AndroidEnvironmentGL> environment);

  ~AndroidContextGL();

  EGLSurface CreateOnscreenSurface(fml::RefPtr<AndroidNativeWindow> window);

  EGLSurface CreateOffscreenSurface();

  void SetEnvironment(fml::RefPtr<AndroidEnvironmentGL>);

  fml::RefPtr<AndroidEnvironmentGL> Environment() const;

  bool IsValid() const;

  bool ClearCurrent();

  bool MakeCurrent(EGLSurface surface);

  bool ResourceMakeCurrent(EGLSurface surface);

  bool SwapBuffers(EGLSurface surface);

  SkISize GetSize(EGLSurface surface);

  bool TeardownSurface(EGLSurface surface);

 private:
  fml::RefPtr<AndroidEnvironmentGL> environment_;
  EGLConfig config_;
  EGLContext context_;
  EGLContext resource_context_;
  bool valid_;

  FML_FRIEND_MAKE_REF_COUNTED(AndroidContextGL);
  FML_FRIEND_REF_COUNTED_THREAD_SAFE(AndroidContextGL);
  FML_DISALLOW_COPY_AND_ASSIGN(AndroidContextGL);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_CONTEXT_GL_H_
