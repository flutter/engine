// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_studio_metal.h"

#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/shell/gpu/gpu_studio_metal_skia.h"
#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"
#import "flutter/shell/platform/darwin/graphics/FlutterDarwinContextMetalSkia.h"
#include "flutter/shell/platform/embedder/embedder_surface_metal.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

FLUTTER_ASSERT_NOT_ARC
namespace flutter {

EmbedderStudioMetal::EmbedderStudioMetal(
    GPUMTLDeviceHandle device,
    GPUMTLCommandQueueHandle command_queue,
    MetalDispatchTable metal_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : GPUSurfaceMetalDelegate(MTLRenderTargetType::kMTLTexture),
      metal_dispatch_table_(std::move(metal_dispatch_table)),
      external_view_embedder_(std::move(external_view_embedder)) {
  main_context_ =
      [FlutterDarwinContextMetalSkia createGrContext:(id<MTLDevice>)device
                                        commandQueue:(id<MTLCommandQueue>)command_queue];
  resource_context_ =
      [FlutterDarwinContextMetalSkia createGrContext:(id<MTLDevice>)device
                                        commandQueue:(id<MTLCommandQueue>)command_queue];
  sksl_precompiler_ = std::make_shared<GPUSurfaceMetalDelegate::SkSLPrecompiler>();
  valid_ = main_context_ && resource_context_;
}

EmbedderStudioMetal::~EmbedderStudioMetal() = default;

bool EmbedderStudioMetal::IsValid() const {
  return valid_;
}

std::unique_ptr<Studio> EmbedderStudioMetal::CreateGPUStudio() API_AVAILABLE(ios(13.0)) {
  if (@available(iOS 13.0, *)) {
  } else {
    return nullptr;
  }
  if (!IsValid()) {
    return nullptr;
  }

  auto studio = std::make_unique<GPUStudioMetalSkia>(this, main_context_, sksl_precompiler_);

  if (!studio->IsValid()) {
    return nullptr;
  }

  return studio;
}

std::unique_ptr<EmbedderSurface> EmbedderStudioMetal::CreateSurface() {
  if (!IsValid()) {
    return nullptr;
  }
  const bool render_to_surface = !external_view_embedder_;
  return std::make_unique<EmbedderSurfaceMetal>(main_context_,
                                                this,  // GPU surface GL delegate
                                                sksl_precompiler_,
                                                render_to_surface  // render to surface
  );
}

sk_sp<GrDirectContext> EmbedderStudioMetal::CreateResourceContext() const {
  return resource_context_;
}

GPUCAMetalLayerHandle EmbedderStudioMetal::GetCAMetalLayer(const SkISize& frame_info) const {
  FML_CHECK(false) << "Only rendering to MTLTexture is supported.";
  return nullptr;
}

bool EmbedderStudioMetal::PresentDrawable(GrMTLHandle drawable) const {
  FML_CHECK(false) << "Only rendering to MTLTexture is supported.";
  return false;
}

GPUMTLTextureInfo EmbedderStudioMetal::GetMTLTexture(int64_t view_id,
                                                     const SkISize& frame_info) const {
  return metal_dispatch_table_.get_texture(view_id, frame_info);
}

bool EmbedderStudioMetal::PresentTexture(GPUMTLTextureInfo texture) const {
  return metal_dispatch_table_.present(texture);
}

}  // namespace flutter
