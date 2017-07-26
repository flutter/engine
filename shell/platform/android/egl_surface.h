// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_EGL_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_EGL_SURFACE_H_

#include <EGL/egl.h>
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/platform/android/android_native_window.h"
#include "flutter/shell/platform/android/egl_utils.h"
#include "lib/ftl/macros.h"
#include "third_party/skia/include/core/SkSize.h"

namespace shell {

class EGLSurface {
 public:
  enum class Type {
    Window,
    PBuffer,
  };

  EGLSurface(EGLDisplay display,
             PlatformView::SurfaceConfig config,
             ftl::RefPtr<AndroidNativeWindow> window);

  ~EGLSurface();

  static EGLResult<EGLConfig> ChooseEGLConfiguration(
      EGLDisplay display,
      PlatformView::SurfaceConfig config,
      EGLSurface::Type type);

  Type GetType() const;

  bool IsValid() const;

  SkISize GetSize() const;

  ::EGLConfig GetEGLConfig() const;

  ::EGLSurface GetEGLSurface() const;

 private:
  const Type type_;
  const ::EGLDisplay display_ = EGL_NO_DISPLAY;
  ::EGLSurface surface_ = nullptr;
  ::EGLConfig config_ = nullptr;
  bool valid_ = false;

  FTL_DISALLOW_COPY_AND_ASSIGN(EGLSurface);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_EGL_SURFACE_H_
