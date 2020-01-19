// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_VULKAN_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_VULKAN_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_vulkan_delegate.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"
#include "flutter/vulkan/vulkan_proc_table.h"

namespace flutter {

class EmbedderSurfaceVulkan final : public EmbedderSurface,
                                    public GPUSurfaceVulkanDelegate {
 public:
  struct VulkanDispatchTable {};

  EmbedderSurfaceVulkan(
      VulkanDispatchTable dispatch_table,
      std::unique_ptr<EmbedderExternalViewEmbedder> external_view_embedder);

  ~EmbedderSurfaceVulkan() override;

  // |EmbedderSurface|
  bool IsValid() const override;

  // |EmbedderSurface|
  std::unique_ptr<Surface> CreateGPUSurface() override;

  // |EmbedderSurface|
  sk_sp<GrContext> CreateResourceContext() const override;

  // |GPUSurfaceVulkanDelegate|
  ExternalViewEmbedder* GetExternalViewEmbedder() override;

  // |GPUSurfaceVulkanDelegate|
  fml::RefPtr<vulkan::VulkanProcTable> vk() override;

 private:
  VulkanDispatchTable dispatch_table_;
  fml::RefPtr<vulkan::VulkanProcTable> proc_table_;
  std::unique_ptr<EmbedderExternalViewEmbedder> external_view_embedder_;
  sk_sp<SkSurface> sk_surface_;
  bool valid_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderSurfaceVulkan);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_VULKAN_H_
