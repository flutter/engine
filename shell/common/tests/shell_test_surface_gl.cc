// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/tests/shell_test_surface_gl.h"

#include "flutter/shell/gpu/gpu_surface_gl.h"

namespace flutter {
namespace testing {

ShellTestSurfaceGL::ShellTestSurfaceGL()
    : gl_surface_(SkISize::Make(800, 600)) {}

ShellTestSurfaceGL::~ShellTestSurfaceGL() = default;

// |GPUSurfaceGLDelegate|
bool ShellTestSurfaceGL::GLContextMakeCurrent() {
  return gl_surface_.MakeCurrent();
}

// |GPUSurfaceGLDelegate|
bool ShellTestSurfaceGL::GLContextClearCurrent() {
  return gl_surface_.ClearCurrent();
}

// |GPUSurfaceGLDelegate|
bool ShellTestSurfaceGL::GLContextPresent() {
  return gl_surface_.Present();
}

// |GPUSurfaceGLDelegate|
intptr_t ShellTestSurfaceGL::GLContextFBO() const {
  return gl_surface_.GetFramebuffer();
}

// |GPUSurfaceGLDelegate|
GPUSurfaceGLDelegate::GLProcResolver ShellTestSurfaceGL::GetGLProcResolver()
    const {
  return [surface = &gl_surface_](const char* name) -> void* {
    return surface->GetProcAddress(name);
  };
}

// |GPUSurfaceGLDelegate|
ExternalViewEmbedder* ShellTestSurfaceGL::GetExternalViewEmbedder() {
  return nullptr;
}

// |ShellTestSurface|
std::unique_ptr<Surface> ShellTestSurfaceGL::CreateRenderingSurface() {
  return std::make_unique<GPUSurfaceGL>(this, true);
}

}  // namespace testing
}  // namespace flutter
