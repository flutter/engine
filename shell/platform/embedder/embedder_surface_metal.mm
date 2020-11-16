// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_surface_metal.h"

#include "flutter/fml/logging.h"
#import "flutter/shell/platform/darwin/graphics/FlutterDarwinContextMetal.h"
#include "fml/logging.h"
#include "shell/gpu/gpu_surface_metal_delegate.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

EmbedderSurfaceMetal::EmbedderSurfaceMetal()
    : GPUSurfaceMetalDelegate(MTLRenderTargetType::kMTLTexture) {
  darwin_metal_context_ = [[FlutterDarwinContextMetal alloc] initWithDefaultMTLDevice];
  valid_ = darwin_metal_context_ != nullptr;
}

EmbedderSurfaceMetal::~EmbedderSurfaceMetal() = default;

// |EmbedderSurface|
bool EmbedderSurfaceMetal::IsValid() const {
  return valid_;
}

// |EmbedderSurface|
std::unique_ptr<Surface> EmbedderSurfaceMetal::CreateGPUSurface() {
  if (!IsValid()) {
    return nullptr;
  }

  auto surface = std::make_unique<GPUSurfaceMetal>(this, darwin_metal_context_.mainContext);

  if (!surface->IsValid()) {
    return nullptr;
  }

  return surface;
}

// |EmbedderSurface|
sk_sp<GrDirectContext> EmbedderSurfaceMetal::CreateResourceContext() const {
  return darwin_metal_context_.resourceContext;
}

// |GPUSurfaceMetalDelegate|
GPUMTLLayerHandle EmbedderSurfaceMetal::GetCAMetalLayer(MTLFrameInfo frame_info) const {
  FML_CHECK(false) << "Only rendering to MTLTexture is supported on macOS";
  return nullptr;
}

// |GPUSurfaceMetalDelegate|
bool EmbedderSurfaceMetal::PresentDrawable(GrMTLHandle drawable) const {
  FML_CHECK(false) << "Only rendering to MTLTexture is supported on macOS";
  return false;
}

// |GPUSurfaceMetalDelegate|
GPUMTLTextureInfo EmbedderSurfaceMetal::GetMTLTexture(MTLFrameInfo frame_info) const {
  // TODO (iskakaushik)
  return {.texture_id = 0, .texture = nullptr};
}

// |GPUSurfaceMetalDelegate|
bool EmbedderSurfaceMetal::PresentTexture(intptr_t texture_id) const {
  // TODO (iskakaushik)
  return false;
}

}  // namespace flutter
