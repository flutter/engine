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
  (void)dispatch_table_;
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

  vulkan::VulkanNativeSurfaceEmbedder::VulkanDispatchTable
      native_dispatch_table = {
          .get_extension_name = []() -> const char* {
            FML_DCHECK(false) << "get_extension_name called";
            return nullptr;
          },
          .get_skia_extension_name = []() -> uint32_t {
            FML_DCHECK(false) << "get_skia_extension_name called";
            return 0;
          },
          .get_size = []() -> SkISize {
            FML_DCHECK(false) << "get_size called";
            return SkISize::MakeEmpty();
          },
          .create_surface_handle = [](vulkan::VulkanProcTable& vk,
                                      const VkInstance& instance) -> VkSurfaceKHR {
            FML_DCHECK(false) << "create_surface_handle called";
            return VK_NULL_HANDLE;
          }};

  auto vulkan_surface_embedder =
      std::make_unique<vulkan::VulkanNativeSurfaceEmbedder>(
          std::move(native_dispatch_table));

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
