// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_studio_metal_impeller.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include "flutter/common/settings.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/mapping.h"
#include "flutter/fml/trace_event.h"
#include "flutter/impeller/display_list/display_list_dispatcher.h"
#include "flutter/impeller/renderer/backend/metal/surface_mtl.h"

static_assert(!__has_feature(objc_arc), "ARC must be disabled.");

namespace flutter {

static std::shared_ptr<impeller::Renderer> CreateImpellerRenderer(
    std::shared_ptr<impeller::Context> context) {
  auto renderer = std::make_shared<impeller::Renderer>(std::move(context));
  if (!renderer->IsValid()) {
    FML_LOG(ERROR) << "Could not create valid Impeller Renderer.";
    return nullptr;
  }
  return renderer;
}

GPUStudioMetalImpeller::GPUStudioMetalImpeller(GPUStudioMetalDelegate* delegate,
                                                 const std::shared_ptr<impeller::Context>& context)
    : delegate_(delegate),
      impeller_renderer_(CreateImpellerRenderer(context)),
      aiks_context_(
          std::make_shared<impeller::AiksContext>(impeller_renderer_ ? context : nullptr)) {}

GPUStudioMetalImpeller::~GPUStudioMetalImpeller() = default;

// |Studio|
bool GPUStudioMetalImpeller::IsValid() {
  return !!aiks_context_;
}

// |Studio|
GrDirectContext* GPUStudioMetalImpeller::GetContext() {
  return nullptr;
}

// |Studio|
std::unique_ptr<GLContextResult> GPUStudioMetalImpeller::MakeRenderContextCurrent() {
  // This backend has no such concept.
  return std::make_unique<GLContextDefaultResult>(true);
}

bool GPUStudioMetalImpeller::AllowsDrawingWhenGpuDisabled() const {
  return delegate_->AllowsDrawingWhenGpuDisabled();
}

// |Studio|
bool GPUStudioMetalImpeller::EnableRasterCache() const {
  return false;
}

// |Studio|
impeller::AiksContext* GPUStudioMetalImpeller::GetAiksContext() const {
  return aiks_context_.get();
}

Studio::StudioData GPUStudioMetalImpeller::GetStudioData() const {
  if (!(last_drawable_ && [last_drawable_ conformsToProtocol:@protocol(CAMetalDrawable)])) {
    return {};
  }
  id<CAMetalDrawable> metal_drawable = static_cast<id<CAMetalDrawable>>(last_drawable_);
  id<MTLTexture> texture = metal_drawable.texture;
  int bytesPerPixel = 0;
  std::string pixel_format;
  switch (texture.pixelFormat) {
    case MTLPixelFormatBGR10_XR:
      bytesPerPixel = 4;
      pixel_format = "MTLPixelFormatBGR10_XR";
      break;
    case MTLPixelFormatBGRA10_XR:
      bytesPerPixel = 8;
      pixel_format = "MTLPixelFormatBGRA10_XR";
      break;
    case MTLPixelFormatBGRA8Unorm:
      bytesPerPixel = 4;
      pixel_format = "MTLPixelFormatBGRA8Unorm";
      break;
    default:
      return {};
  }

  // Zero initialized so that errors are easier to find at the cost of
  // performance.
  sk_sp<SkData> result =
      SkData::MakeZeroInitialized(texture.width * texture.height * bytesPerPixel);
  [texture getBytes:result->writable_data()
        bytesPerRow:texture.width * bytesPerPixel
         fromRegion:MTLRegionMake2D(0, 0, texture.width, texture.height)
        mipmapLevel:0];
  return {
      .pixel_format = pixel_format,
      .data = result,
  };
}

}  // namespace flutter
