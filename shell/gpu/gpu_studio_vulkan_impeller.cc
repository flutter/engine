// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_studio_vulkan_impeller.h"

#include "flutter/fml/make_copyable.h"
#include "flutter/impeller/display_list/display_list_dispatcher.h"
#include "flutter/impeller/renderer/renderer.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"

namespace flutter {

GPUStudioVulkanImpeller::GPUStudioVulkanImpeller(
    std::shared_ptr<impeller::Context> context)
    : weak_factory_(this) {
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

  impeller_context_ = std::move(context);
  impeller_renderer_ = std::move(renderer);
  aiks_context_ = std::move(aiks_context);
  is_valid_ = true;
}

// |Studio|
GPUStudioVulkanImpeller::~GPUStudioVulkanImpeller() = default;

// |Studio|
bool GPUStudioVulkanImpeller::IsValid() {
  return is_valid_;
}

// |Studio|
GrDirectContext* GPUStudioVulkanImpeller::GetContext() {
  // Impeller != Skia.
  return nullptr;
}

// |Studio|
std::unique_ptr<GLContextResult>
GPUStudioVulkanImpeller::MakeRenderContextCurrent() {
  // This backend has no such concept.
  return std::make_unique<GLContextDefaultResult>(true);
}

// |Studio|
bool GPUStudioVulkanImpeller::EnableRasterCache() const {
  return false;
}

// |Studio|
impeller::AiksContext* GPUStudioVulkanImpeller::GetAiksContext() const {
  return aiks_context_.get();
}

}  // namespace flutter
