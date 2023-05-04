// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_METAL_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_METAL_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"
#include "flutter/shell/gpu/gpu_surface_metal_skia.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_studio_metal.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"

#include "third_party/skia/include/core/SkSurface.h"

namespace flutter {

class EmbedderSurfaceMetal final : public EmbedderSurface {
 public:
  EmbedderSurfaceMetal(sk_sp<GrDirectContext> main_context,
                       EmbedderStudioMetal* studio,
                       std::shared_ptr<GPUSurfaceMetalDelegate::SkSLPrecompiler>
                           sksl_precompiler,
                       bool render_to_surface);

  ~EmbedderSurfaceMetal() override;

  // |EmbedderSurface|
  bool IsValid() const override;

 private:
  sk_sp<GrDirectContext> main_context_;
  EmbedderStudioMetal* studio_;
  std::shared_ptr<GPUSurfaceMetalDelegate::SkSLPrecompiler> sksl_precompiler_;
  bool render_to_surface_;

  // |EmbedderSurface|
  std::unique_ptr<Surface> CreateGPUSurface() override;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderSurfaceMetal);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_METAL_H_
