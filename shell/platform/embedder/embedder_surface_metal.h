// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_METAL_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_METAL_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_metal.h"
#import "flutter/shell/platform/darwin/graphics/FlutterDarwinContextMetal.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"

namespace flutter {

class EmbedderSurfaceMetal final : public EmbedderSurface,
                                   public GPUSurfaceMetalDelegate {
 public:
  explicit EmbedderSurfaceMetal();

  ~EmbedderSurfaceMetal() override;

 private:
  bool valid_ = false;
  FlutterDarwinContextMetal* darwin_metal_context_;
  sk_sp<SkSurface> sk_surface_;

  // |EmbedderSurface|
  bool IsValid() const override;

  // |EmbedderSurface|
  std::unique_ptr<Surface> CreateGPUSurface() override;

  // |EmbedderSurface|
  sk_sp<GrDirectContext> CreateResourceContext() const override;

  // |GPUSurfaceMetalDelegate|
  GPUMTLLayerHandle GetCAMetalLayer(MTLFrameInfo frame_info) const override;

  // |GPUSurfaceMetalDelegate|
  bool PresentDrawable(GrMTLHandle drawable) const override;

  // |GPUSurfaceMetalDelegate|
  GPUMTLTextureInfo GetMTLTexture(MTLFrameInfo frame_info) const override;

  // |GPUSurfaceMetalDelegate|
  bool PresentTexture(intptr_t texture_id) const override;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderSurfaceMetal);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_METAL_H_
