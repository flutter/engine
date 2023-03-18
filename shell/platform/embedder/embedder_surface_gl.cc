// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_surface_gl.h"

#include <utility>

#include "flutter/shell/common/shell_io_manager.h"
#include "flutter/shell/platform/embedder/embedder_studio_gl.h"

namespace flutter {

EmbedderSurfaceGL::EmbedderSurfaceGL(sk_sp<GrDirectContext> main_context,
                                     EmbedderStudioGL* studio,
                                     bool render_to_surface)
    : main_context_(std::move(main_context)),
      studio_(studio),
      render_to_surface_(render_to_surface) {}

EmbedderSurfaceGL::~EmbedderSurfaceGL() {}

// |EmbedderSurface|
bool EmbedderSurfaceGL::IsValid() const {
  return studio_->IsValid();
}

// |EmbedderSurface|
std::unique_ptr<Surface> EmbedderSurfaceGL::CreateGPUSurface() {
  if (!IsValid()) {
    return nullptr;
  }
  return std::make_unique<GPUSurfaceGLSkia>(
      main_context_,
      studio_,            // GPU surface GL delegate
      render_to_surface_  // render to surface
  );
}

// |EmbedderSurface|
sk_sp<GrDirectContext> EmbedderSurfaceGL::CreateResourceContext() const {
  return studio_->CreateResourceContext();
}

}  // namespace flutter
