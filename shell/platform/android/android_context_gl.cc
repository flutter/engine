// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_context_gl.h"
#include <utility>
#include "flutter/shell/platform/android/egl_utils.h"

namespace shell {

static EGLResult<EGLContext> CreateContext(EGLDisplay display,
                                           EGLConfig config,
                                           EGLContext share = EGL_NO_CONTEXT) {
  EGLint attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

  ::EGLContext context = eglCreateContext(display, config, share, attributes);

  return {context != EGL_NO_CONTEXT, context};
}

AndroidContextGL::AndroidContextGL(ftl::RefPtr<AndroidEnvironmentGL> env,
                                   PlatformView::SurfaceConfig config,
                                   ftl::RefPtr<AndroidNativeWindow> window,
                                   const AndroidContextGL* share_context)
    : environment_(env), surface_config_(config), window_(std::move(window)) {
  if (!environment_->IsValid()) {
    return;
  }

  bool success = false;
  ::EGLConfig window_config = nullptr;

  // Choose a valid window configuration.
  std::tie(success, window_config) = EGLSurface::ChooseEGLConfiguration(
      environment_->Display(), config, EGLSurface::Type::Window);

  if (!success) {
    FTL_LOG(ERROR) << "Could not choose a valid EGL window configuration.";
    LogLastEGLError();
    return;
  }

  // Create a context for the configuration.
  std::tie(success, context_) = CreateContext(
      environment_->Display(), window_config,
      share_context != nullptr ? share_context->context_ : EGL_NO_CONTEXT);

  if (!success) {
    FTL_LOG(ERROR) << "Could not create an EGL context";
    LogLastEGLError();
    return;
  }

  auto surface =
      std::make_unique<EGLSurface>(environment_->Display(), config, window_);
  if (surface == nullptr || !surface->IsValid()) {
    FTL_LOG(ERROR) << "Could not create an EGL Pbuffer context";
  }
  surface_ = std::move(surface);

  // All done!
  valid_ = true;
}

AndroidContextGL::~AndroidContextGL() {
  if (context_ == EGL_NO_CONTEXT) {
    return;
  }

  if (eglDestroyContext(environment_->Display(), context_) != EGL_TRUE) {
    FTL_LOG(ERROR)
        << "Could not tear down the EGL context. Possible resource leak.";
    LogLastEGLError();
  }
}

ftl::RefPtr<AndroidEnvironmentGL> AndroidContextGL::Environment() const {
  return environment_;
}

bool AndroidContextGL::IsValid() const {
  return valid_;
}

bool AndroidContextGL::MakeCurrent() {
  FTL_DCHECK(surface_ && surface_->IsValid());
  ::EGLSurface surface = surface_->GetEGLSurface();
  if (::eglMakeCurrent(environment_->Display(), surface, surface, context_) !=
      EGL_TRUE) {
    FTL_LOG(ERROR) << "Could not make the context current";
    LogLastEGLError();
    return false;
  }
  return true;
}

bool AndroidContextGL::ClearCurrent() {
  if (::eglMakeCurrent(environment_->Display(), nullptr, nullptr,
                       EGL_NO_CONTEXT) != EGL_TRUE) {
    FTL_LOG(ERROR) << "Could not clear the current context";
    LogLastEGLError();
    return false;
  }
  return true;
}

bool AndroidContextGL::SwapBuffers() {
  TRACE_EVENT0("flutter", "AndroidContextGL::SwapBuffers");
  return eglSwapBuffers(environment_->Display(), surface_->GetEGLSurface());
}

SkISize AndroidContextGL::GetSize() {
  return surface_->GetSize();
}

bool AndroidContextGL::Resize(const SkISize& size) {
  if (size == GetSize()) {
    return true;
  }

  ClearCurrent();

  // Either way, the old surface needs to go.
  surface_ = nullptr;
  valid_ = false;

  auto new_surface = std::make_unique<EGLSurface>(environment_->Display(),
                                                  surface_config_, window_);
  if (!new_surface->IsValid() ||
      new_surface->GetType() != EGLSurface::Type::Window) {
    FTL_LOG(INFO) << "Could not resize the OpenGL surface.";
    return false;
  }

  surface_ = std::move(new_surface);
  valid_ = true;

  MakeCurrent();

  return true;
}

}  // namespace shell
