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

GPUStudioMetalImpeller::GPUStudioMetalImpeller(GPUSurfaceMetalDelegate* delegate,
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
std::shared_ptr<impeller::AiksContext> GPUStudioMetalImpeller::GetAiksContext() const {
  return aiks_context_;
}

}  // namespace flutter
