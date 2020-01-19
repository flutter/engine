// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_surface_vulkan.h"

#include "flutter/fml/trace_event.h"
#include "flutter/shell/gpu/gpu_surface_vulkan.h"
#include "flutter/vulkan/vulkan_native_surface_embedder.h"
#include "flutter/vulkan/vulkan_proc_table.h"

namespace flutter {

EmbedderSurfaceVulkan::EmbedderSurfaceVulkan(
    VulkanDispatchTable dispatch_table,
    std::unique_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : dispatch_table_(dispatch_table),
      proc_table_(fml::MakeRefCounted<vulkan::VulkanProcTable>()),
      external_view_embedder_(std::move(external_view_embedder)) {
  valid_ = true;
}

EmbedderSurfaceVulkan::~EmbedderSurfaceVulkan() = default;

bool EmbedderSurfaceVulkan::IsValid() const {
  return valid_;
}

std::unique_ptr<Surface> EmbedderSurfaceVulkan::CreateGPUSurface() {
  if (!IsValid()) {
    return nullptr;
  }

  auto vulkan_surface_embedder =
      std::make_unique<vulkan::VulkanNativeSurfaceEmbedder>(dispatch_table_);

  if (!vulkan_surface_embedder->IsValid()) {
    return nullptr;
  }

  const bool render_to_surface = !external_view_embedder_;
  auto surface = std::make_unique<GPUSurfaceVulkan>(
      this, std::move(vulkan_surface_embedder), render_to_surface);

  if (!surface->IsValid()) {
    return nullptr;
  }

  return surface;
}

sk_sp<GrContext> EmbedderSurfaceVulkan::CreateResourceContext() const {
  return nullptr;
}

ExternalViewEmbedder* EmbedderSurfaceVulkan::GetExternalViewEmbedder() {
  return external_view_embedder_.get();
}

fml::RefPtr<vulkan::VulkanProcTable> EmbedderSurfaceVulkan::vk() {
  return proc_table_;
}

}  // namespace flutter
