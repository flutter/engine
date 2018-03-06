// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_surface_gl.h"

#include "flutter/shell/gpu/gpu_surface_gl.h"
#include "flutter/shell/platform/darwin/ios/ios_system_compositor_context.h"

namespace shell {

IOSSurfaceGL::IOSSurfaceGL(PlatformView::SurfaceConfig surface_config,
                           CAEAGLLayer* layer,
                           EAGLContext* eaglContext)
    : IOSSurface(surface_config, reinterpret_cast<CALayer*>(layer)),
      context_(surface_config, layer, eaglContext) {}

IOSSurfaceGL::~IOSSurfaceGL() = default;

bool IOSSurfaceGL::IsValid() const {
  return context_.IsValid();
}

bool IOSSurfaceGL::ResourceContextMakeCurrent() {
  return IsValid() ? context_.ResourceMakeCurrent() : false;
}

void IOSSurfaceGL::UpdateStorageSizeIfNecessary() {
  if (IsValid()) {
    context_.UpdateStorageSizeIfNecessary();
  }
}

std::unique_ptr<GPUSurfaceGL> IOSSurfaceGL::CreateGPUSurface(GrContext* grContext) {
  return std::make_unique<GPUSurfaceGL>(this, grContext);
}

intptr_t IOSSurfaceGL::GLContextFBO() const {
  return IsValid() ? context_.framebuffer() : GL_NONE;
}

bool IOSSurfaceGL::UseOffscreenSurface() const {
  // The onscreen surface wraps a GL renderbuffer, which is extremely slow to read.
  // Certain filter effects require making a copy of the current destination, so we
  // always render to an offscreen surface, which will be much quicker to read/copy.
  return true;
}

bool IOSSurfaceGL::GLContextMakeCurrent() {
  return IsValid() ? context_.MakeCurrent() : false;
}

bool IOSSurfaceGL::GLContextClearCurrent() {
  [EAGLContext setCurrentContext:nil];
  return true;
}

bool IOSSurfaceGL::GLContextPresent() {
  TRACE_EVENT0("flutter", "IOSSurfaceGL::GLContextPresent");
  if (IsValid()) {
    return context_.PresentRenderBuffer();
  } else {
    return false;
  }
}

}  // namespace shell
