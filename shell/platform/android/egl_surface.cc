// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/egl_surface.h"

namespace shell {

EGLResult<::EGLConfig> EGLSurface::ChooseEGLConfiguration(
    EGLDisplay display,
    PlatformView::SurfaceConfig config,
    EGLSurface::Type type) {
  EGLint surface_type = EGL_PBUFFER_BIT;

  switch (type) {
    case EGLSurface::Type::Window:
      surface_type = EGL_WINDOW_BIT;
      break;
    case EGLSurface::Type::PBuffer:
      surface_type = EGL_PBUFFER_BIT;
      break;
  }

  EGLint attributes[] = {
      // clang-format off
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_SURFACE_TYPE,    surface_type,
      EGL_RED_SIZE,        config.red_bits,
      EGL_GREEN_SIZE,      config.green_bits,
      EGL_BLUE_SIZE,       config.blue_bits,
      EGL_ALPHA_SIZE,      config.alpha_bits,
      EGL_DEPTH_SIZE,      config.depth_bits,
      EGL_STENCIL_SIZE,    config.stencil_bits,
      EGL_NONE,            // termination sentinel
      // clang-format on
  };

  EGLint config_count = 0;
  EGLConfig egl_config = nullptr;

  if (::eglChooseConfig(display, attributes, &egl_config, 1, &config_count) !=
      EGL_TRUE) {
    return {false, nullptr};
  }

  bool success = config_count > 0 && egl_config != nullptr;

  return {success, success ? egl_config : nullptr};
}

static EGLResult<::EGLSurface> CreateSurface(
    EGLDisplay display,
    EGLConfig config,
    ftl::RefPtr<AndroidNativeWindow> window) {
  ::EGLSurface surface = nullptr;

  if (window) {
    EGLNativeWindowType native_window =
        reinterpret_cast<EGLNativeWindowType>(window->handle());

    surface = eglCreateWindowSurface(display,        // display
                                     config,         // config
                                     native_window,  // native window
                                     nullptr         // attribute list
                                     );
  } else {
    // We only ever create pbuffer surfaces for background resource loading
    // contexts. We never bind the pbuffer to anything. But the pbuffers must
    // have a size. So specify 1x1.
    const EGLint attributes[] = {
        EGL_WIDTH, 1,   //
        EGL_HEIGHT, 1,  //
        EGL_NONE        // termination sentinel
    };

    surface = eglCreatePbufferSurface(display,    // display
                                      config,     // config
                                      attributes  // attribute list
                                      );
  }

  bool success = surface != nullptr;
  return {success, success ? surface : nullptr};
}

EGLSurface::EGLSurface(EGLDisplay display,
                       PlatformView::SurfaceConfig config,
                       ftl::RefPtr<AndroidNativeWindow> window)
    : type_(window ? Type::PBuffer : Type::Window), display_(display) {
  if (display == EGL_NO_DISPLAY) {
    return;
  }

  bool success = false;

  std::tie(success, config_) = ChooseEGLConfiguration(display, config, type_);

  if (!success) {
    FTL_LOG(ERROR) << "Could not choose EGL configuration.";
    LogLastEGLError();
    return;
  }

  std::tie(success, surface_) = CreateSurface(display, config_, window);

  if (!success) {
    FTL_LOG(ERROR) << "Could not create EGL surface.";
    LogLastEGLError();
    return;
  }

  valid_ = true;
}

EGLSurface::~EGLSurface() {
  if (surface_ == nullptr) {
    return;
  }

  auto result = ::eglDestroySurface(display_, surface_);
  if (result != EGL_TRUE) {
    FTL_LOG(ERROR)
        << "Could not destroy the EGL surface. This is a resource leak.";
  }
}

EGLSurface::Type EGLSurface::GetType() const {
  return type_;
}

bool EGLSurface::IsValid() const {
  return valid_;
}

SkISize EGLSurface::GetSize() const {
  if (!valid_) {
    return SkISize::Make(0, 0);
  }

  EGLint width = 0;
  EGLint height = 0;

  if (!::eglQuerySurface(display_, surface_, EGL_WIDTH, &width) ||
      !::eglQuerySurface(display_, surface_, EGL_HEIGHT, &height)) {
    FTL_LOG(ERROR) << "Unable to query EGL surface size";
    LogLastEGLError();
    return SkISize::Make(0, 0);
  }

  return SkISize::Make(width, height);
}

EGLConfig EGLSurface::GetEGLConfig() const {
  return config_;
}

::EGLSurface EGLSurface::GetEGLSurface() const {
  return surface_;
}

}  // namespace shell
