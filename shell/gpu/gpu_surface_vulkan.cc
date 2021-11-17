// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_surface_vulkan.h"

#include "flutter/fml/logging.h"
#include "fml/trace_event.h"
#include "include/core/SkSize.h"

namespace flutter {

GPUSurfaceVulkan::GPUSurfaceVulkan(GPUSurfaceVulkanDelegate* delegate,
                                   const sk_sp<GrDirectContext>& skia_context,
                                   bool render_to_surface)
    : delegate_(delegate),
      skia_context_(skia_context),
      render_to_surface_(render_to_surface),
      weak_factory_(this) {}

GPUSurfaceVulkan::~GPUSurfaceVulkan() = default;

bool GPUSurfaceVulkan::IsValid() {
  return skia_context_ != nullptr;
}

std::unique_ptr<SurfaceFrame> GPUSurfaceVulkan::AcquireFrame(
    const SkISize& frame_size) {
  if (!IsValid()) {
    FML_LOG(ERROR) << "Vulkan surface was invalid.";
    return nullptr;
  }

  if (frame_size.isEmpty()) {
    FML_LOG(ERROR) << "Vulkan surface was asked for an empty frame.";
    return nullptr;
  }

  if (!render_to_surface_) {
    return std::make_unique<SurfaceFrame>(
        nullptr, SurfaceFrame::FramebufferInfo(),
        [](const SurfaceFrame& surface_frame, SkCanvas* canvas) {
          return true;
        });
  }

  VkImage image = delegate_->AcquireImage(frame_size);
  if (!image) {
    FML_LOG(ERROR) << "Invalid VkImage given by the embedder.";
    return nullptr;
  }

  sk_sp<SkSurface> surface = CreateSurfaceFromVulkanImage(image, frame_size);
  if (!surface) {
    FML_LOG(ERROR) << "Could not create the SkSurface from the Vulkan image.";
    return nullptr;
  }

  SurfaceFrame::SubmitCallback callback = [image = image, delegate = delegate_](
                                              const SurfaceFrame&,
                                              SkCanvas* canvas) -> bool {
    TRACE_EVENT0("flutter", "GPUSurfaceVulkan::PresentImage");
    if (canvas == nullptr) {
      FML_DLOG(ERROR) << "Canvas not available.";
      return false;
    }

    canvas->flush();

    return delegate->PresentImage(image);
  };

  SurfaceFrame::FramebufferInfo framebuffer_info{.supports_readback = true};

  return std::make_unique<SurfaceFrame>(
      std::move(surface), std::move(framebuffer_info), std::move(callback));
}

SkMatrix GPUSurfaceVulkan::GetRootTransformation() const {
  // This backend does not support delegating to the underlying platform to
  // query for root surface transformations. Just return identity.
  SkMatrix matrix;
  matrix.reset();
  return matrix;
}

GrDirectContext* GPUSurfaceVulkan::GetContext() {
  return skia_context_.get();
}

sk_sp<SkSurface> GPUSurfaceVulkan::CreateSurfaceFromVulkanImage(
    VkImage image,
    const SkISize& size) {
  GrVkImageInfo image_info = {
      .fImage = image,
      .fImageTiling = VK_IMAGE_TILING_OPTIMAL,
      .fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .fFormat = VK_FORMAT_R8G8B8A8_UNORM,
      .fImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                          VK_IMAGE_USAGE_SAMPLED_BIT,
      .fSampleCount = 1,
      .fLevelCount = 1,
  };
  GrBackendTexture backend_texture(size.width(),   //
                                   size.height(),  //
                                   image_info      //
  );

  SkSurfaceProps surface_properties(0, kUnknown_SkPixelGeometry);

  return SkSurface::MakeFromBackendTexture(
      skia_context_.get(),             // context
      backend_texture,           // back-end texture
      kTopLeft_GrSurfaceOrigin,  // surface origin
      1,                         // sample count
      kRGBA_8888_SkColorType,    // color type
      SkColorSpace::MakeSRGB(),  // color space
      &surface_properties        // surface properties
  );
}

}  // namespace flutter
