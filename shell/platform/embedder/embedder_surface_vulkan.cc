// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_surface_vulkan.h"

#include <utility>

#include "flutter/flutter_vma/flutter_skia_vma.h"
#include "flutter/shell/common/shell_io_manager.h"
#include "flutter/shell/gpu/gpu_surface_vulkan.h"
#include "flutter/shell/gpu/gpu_surface_vulkan_delegate.h"
#include "flutter/vulkan/vulkan_skia_proc_table.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"

namespace flutter {

EmbedderSurfaceVulkan::EmbedderSurfaceVulkan(
    EmbedderStudioVulkan* studio,
    sk_sp<GrDirectContext> main_context,
    bool render_to_surface)
    : main_context_(std::move(main_context)),
      studio_(studio),
      render_to_surface_(render_to_surface) {}

EmbedderSurfaceVulkan::~EmbedderSurfaceVulkan() {}

// |EmbedderSurface|
bool EmbedderSurfaceVulkan::IsValid() const {
  return studio_->IsValid();
}

// |EmbedderSurface|
std::unique_ptr<Surface> EmbedderSurfaceVulkan::CreateGPUSurface() {
  return std::make_unique<GPUSurfaceVulkan>(studio_, main_context_,
                                            render_to_surface_);
}

// |EmbedderSurface|
sk_sp<GrDirectContext> EmbedderSurfaceVulkan::CreateResourceContext() const {
  return studio_->CreateResourceContext();
}

}  // namespace flutter
