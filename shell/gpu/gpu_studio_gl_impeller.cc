// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_studio_gl_impeller.h"

#include "flutter/fml/make_copyable.h"
#include "flutter/impeller/display_list/display_list_dispatcher.h"
#include "flutter/impeller/renderer/backend/gles/surface_gles.h"
#include "flutter/impeller/renderer/renderer.h"

namespace flutter {

GPUStudioGLImpeller::GPUStudioGLImpeller(
    GPUSurfaceGLDelegate* delegate,
    const std::shared_ptr<impeller::Context>& context) {
  if (delegate == nullptr) {
    return;
  }

  if (!context || !context->IsValid()) {
    return;
  }

  auto renderer = std::make_shared<impeller::Renderer>(context);
  if (!renderer->IsValid()) {
    return;
  }

  auto aiks_context = std::make_shared<impeller::AiksContext>(context);

  if (!aiks_context->IsValid()) {
    return;
  }

  delegate_ = delegate;
  aiks_context_ = std::move(aiks_context);
  is_valid_ = true;
}

// |Studio|
GPUStudioGLImpeller::~GPUStudioGLImpeller() = default;

// |Studio|
bool GPUStudioGLImpeller::IsValid() {
  return is_valid_;
}

// |Studio|
GrDirectContext* GPUStudioGLImpeller::GetContext() {
  // Impeller != Skia.
  return nullptr;
}

// |Studio|
std::unique_ptr<GLContextResult>
GPUStudioGLImpeller::MakeRenderContextCurrent() {
  return delegate_->GLContextMakeCurrent();
}

// |Studio|
bool GPUStudioGLImpeller::ClearRenderContext() {
  return delegate_->GLContextClearCurrent();
}

bool GPUStudioGLImpeller::AllowsDrawingWhenGpuDisabled() const {
  return delegate_->AllowsDrawingWhenGpuDisabled();
}

// |Studio|
bool GPUStudioGLImpeller::EnableRasterCache() const {
  return false;
}

// |Studio|
std::shared_ptr<impeller::AiksContext> GPUStudioGLImpeller::GetAiksContext() const {
  return aiks_context_;
}

}  // namespace flutter
