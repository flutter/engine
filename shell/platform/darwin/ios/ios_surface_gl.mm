// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_surface_gl.h"

#include "flutter/shell/gpu/gpu_surface_gl.h"
#include "flutter/shell/platform/darwin/ios/ios_layered_paint_context.h"

namespace shell {

IOSSurfaceGL::IOSSurfaceGL(PlatformView::SurfaceConfig surface_config, CAEAGLLayer* layer)
    : IOSSurface(surface_config, reinterpret_cast<CALayer*>(layer)),
      context_(surface_config, layer) {}

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

std::unique_ptr<GPUSurfaceGL> IOSSurfaceGL::CreateGPUSurface() {
  return std::make_unique<GPUSurfaceGL>(this);
}

intptr_t IOSSurfaceGL::GLContextFBO() const {
  return IsValid() ? context_.framebuffer() : GL_NONE;
}

bool IOSSurfaceGL::GLContextMakeCurrent() {
  return IsValid() ? context_.MakeCurrent() : false;
}

bool IOSSurfaceGL::GLContextMakeCurrent2() {
  return IsValid() ? context_.MakeCurrent2() : false;
}

bool IOSSurfaceGL::GLContextClearCurrent() {
  [EAGLContext setCurrentContext:nil];
  return true;
}

flow::LayeredPaintContext* IOSSurfaceGL::CreateLayeredPaintContext() {
  layered_paint_context_ = new IOSLayeredPaintContext(this);
  return layered_paint_context_;
}

bool IOSSurfaceGL::GLContextPresent() {
  TRACE_EVENT0("flutter", "IOSSurfaceGL::GLContextPresent");
  if (IsValid()) {
    bool result = true;
    if (layered_paint_context_) {
      layered_paint_context_->Finish();
    } else {
      result = context_.PresentRenderBuffer();
    }
    return result;
  } else {
    return false;
  }
}

}  // namespace shell
