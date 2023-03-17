// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_GL_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_GL_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_gl_skia.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_studio_gl.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"

namespace flutter {

class EmbedderSurfaceGL final : public EmbedderSurface {
 public:
  EmbedderSurfaceGL(sk_sp<GrDirectContext> main_context,
                    EmbedderStudioGL* studio,
                    bool render_to_surface);

  ~EmbedderSurfaceGL() override;

  // |EmbedderSurface|
  bool IsValid() const override;

 private:
  sk_sp<GrDirectContext> main_context_;
  EmbedderStudioGL* studio_;
  bool render_to_surface_;

  // |EmbedderSurface|
  std::unique_ptr<Surface> CreateGPUSurface() override;

  // |EmbedderSurface|
  sk_sp<GrDirectContext> CreateResourceContext() const override;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderSurfaceGL);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_GL_H_
