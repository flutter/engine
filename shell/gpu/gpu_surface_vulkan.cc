// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_surface_vulkan.h"

#include "flutter/fml/logging.h"

namespace flutter {

GPUSurfaceVulkan::GPUSurfaceVulkan(const sk_sp<GrDirectContext>& skia_context,
                                   GPUSurfaceVulkanDelegate* delegate)
    : skia_context_(skia_context), delegate_(delegate), weak_factory_(this) {}

GPUSurfaceVulkan::~GPUSurfaceVulkan() = default;

bool GPUSurfaceVulkan::IsValid() {
  return image_ != nullptr;
}

std::unique_ptr<SurfaceFrame> GPUSurfaceVulkan::AcquireFrame(
    const SkISize& size) {
  VkImage image = delegate_->AcquireImage(size);

  // TODO(38466): Refactor GPU surface APIs take into account the fact that an
  // external view embedder may want to render to the root surface.
  if (!render_to_surface_) {
    return std::make_unique<SurfaceFrame>(
        nullptr, std::move(framebuffer_info),
        [](const SurfaceFrame& surface_frame, SkCanvas* canvas) {
          return true;
        });
  }

  sk_sp<SkSurface> surface = window_.AcquireSurface();

  if (surface == nullptr) {
    return nullptr;
  }

  SurfaceFrame::SubmitCallback callback =
      [weak_this = weak_factory_.GetWeakPtr()](const SurfaceFrame&,
                                               SkCanvas* canvas) -> bool {
    // Frames are only ever acquired on the raster thread. This is also the
    // thread on which the weak pointer factory is collected (as this instance
    // is owned by the rasterizer). So this use of weak pointers is safe.
    if (canvas == nullptr || !weak_this) {
      return false;
    }
    return weak_this->Present();
  };
  return std::make_unique<SurfaceFrame>(
      std::move(surface), std::move(framebuffer_info), std::move(callback));
}

bool GPUSurfaceVulkan::Present() {}

SkMatrix GPUSurfaceVulkan::GetRootTransformation() const {
  // This backend does not support delegating to the underlying platform to
  // query for root surface transformations. Just return identity.
  SkMatrix matrix;
  matrix.reset();
  return matrix;
}

GrDirectContext* GPUSurfaceVulkan::GetContext() {
  return skia_context_;
}

sk_sp<SkSurface> GPUSurfaceVulkan::AcquireSurfaceFromVulkanImage(
    VkImage image) {

}

}  // namespace flutter
