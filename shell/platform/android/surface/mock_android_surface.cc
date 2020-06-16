// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/surface/mock_android_surface.h"

#include "flutter/shell/gpu/gpu_surface_gl.h"

namespace flutter {

MockAndroidSurface::MockAndroidSurface(int id) : id_(id){};

MockAndroidSurface::~MockAndroidSurface() = default;

bool MockAndroidSurface::IsValid() const {
  return true;
}

void MockAndroidSurface::TeardownOnScreenContext() {}

std::unique_ptr<Surface> MockAndroidSurface::CreateGPUSurface(
    GrContext* gr_context) {
  if (gr_context) {
    return std::make_unique<GPUSurfaceGL>(sk_ref_sp(gr_context), this, true);
  }
  return std::make_unique<GPUSurfaceGL>(this, true);
}

bool MockAndroidSurface::OnScreenSurfaceResize(const SkISize& size) {
  return true;
}

bool MockAndroidSurface::ResourceContextMakeCurrent() {
  return true;
}

bool MockAndroidSurface::ResourceContextClearCurrent() {
  return true;
}

bool MockAndroidSurface::SetNativeWindow(
    fml::RefPtr<AndroidNativeWindow> window) {
  return true;
}

std::unique_ptr<GLContextResult> MockAndroidSurface::GLContextMakeCurrent() {
  return std::make_unique<GLContextDefaultResult>(/*static_result=*/true);
}

bool MockAndroidSurface::GLContextClearCurrent() {
  return true;
}

bool MockAndroidSurface::GLContextPresent() {
  return true;
}

intptr_t MockAndroidSurface::GLContextFBO() const {
  return 0;
}

ExternalViewEmbedder* MockAndroidSurface::GetExternalViewEmbedder() {
  return nullptr;
}

int MockAndroidSurface::GetId() const {
  return id_;
}

}  // namespace flutter
