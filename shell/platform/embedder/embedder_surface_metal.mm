// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "flutter/shell/platform/embedder/embedder_surface_metal.h"

#include "flutter/fml/logging.h"
#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"
#import "flutter/shell/platform/darwin/graphics/FlutterDarwinContextMetalSkia.h"
#include "flutter/shell/platform/embedder/embedder_studio_metal.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

FLUTTER_ASSERT_NOT_ARC
namespace flutter {

EmbedderSurfaceMetal::EmbedderSurfaceMetal(
    sk_sp<GrDirectContext> main_context,
    EmbedderStudioMetal* studio,
    std::shared_ptr<GPUSurfaceMetalDelegate::SkSLPrecompiler> sksl_precompiler,
    bool render_to_surface)
    : main_context_(std::move(main_context)),
      studio_(studio),
      sksl_precompiler_(std::move(sksl_precompiler)),
      render_to_surface_(render_to_surface) {}

EmbedderSurfaceMetal::~EmbedderSurfaceMetal() = default;

bool EmbedderSurfaceMetal::IsValid() const {
  return studio_->IsValid();
}

std::unique_ptr<Surface> EmbedderSurfaceMetal::CreateGPUSurface() API_AVAILABLE(ios(13.0)) {
  if (@available(iOS 13.0, *)) {
  } else {
    return nullptr;
  }
  if (!IsValid()) {
    return nullptr;
  }

  auto surface = std::make_unique<GPUSurfaceMetalSkia>(
      studio_, main_context_, MsaaSampleCount::kNone, sksl_precompiler_, render_to_surface_);

  if (!surface->IsValid()) {
    return nullptr;
  }

  return surface;
}

}  // namespace flutter
