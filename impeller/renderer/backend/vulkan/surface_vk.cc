// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/surface_vk.h"
#include <memory>

#include "fml/logging.h"
#include "impeller/renderer/backend/vulkan/formats_vk.h"
#include "impeller/renderer/surface.h"
#include "vulkan/vulkan_handles.hpp"

namespace impeller {
std::unique_ptr<Surface> SurfaceVK::WrapSurface(
    ContextVK* context,
    vk::SurfaceKHR surface,
    std::shared_ptr<SwapchainVK> swapchain) {
  if (context == nullptr || !context->IsValid() || swapchain == nullptr) {
    return nullptr;
  }

  const auto color_format = FromVKFormat(swapchain->image_format_);

  if (color_format == PixelFormat::kUnknown) {
    VALIDATION_LOG << "Unknown drawable color format.";
    return nullptr;
  }

  TextureDescriptor color0_tex_desc;
  color0_tex_desc.type = TextureType::kTexture2D;
  color0_tex_desc.sample_count = SampleCount::kCount1;
  color0_tex_desc.format = color_format;
  color0_tex_desc.size = {static_cast<ISize::Type>(swapchain->extent_.width),
                          static_cast<ISize::Type>(swapchain->extent_.height)};
  color0_tex_desc.usage = static_cast<uint64_t>(TextureUsage::kRenderTarget);

  auto msaa_tex = context->GetPermanentsAllocator()->CreateTexture(
      StorageMode::kDeviceTransient, color0_tex_desc);
  if (!msaa_tex) {
    VALIDATION_LOG << "Could not allocate MSAA resolve texture.";
    return nullptr;
  }
  msaa_tex->SetLabel("ImpellerOnscreenColor");

  ColorAttachment color0;
  color0.texture = msaa_tex;
  color0.clear_color = Color::DarkSlateGray();
  color0.load_action = LoadAction::kClear;
  color0.store_action = StoreAction::kDontCare;

  RenderTarget render_target_desc;
  render_target_desc.SetColorAttachment(color0, 0u);

  return std::make_unique<SurfaceVK>(render_target_desc, surface, swapchain);
}

SurfaceVK::SurfaceVK(RenderTarget target,
                     vk::SurfaceKHR surface,
                     std::shared_ptr<SwapchainVK> swapchain)
    : Surface(target), surface_(surface), swapchain_(swapchain) {
  FML_DCHECK(surface_);
}

vk::SurfaceKHR SurfaceVK::GetSurface() const {
  return surface_;
}

SurfaceVK::~SurfaceVK() = default;

bool SurfaceVK::Present() const {
  FML_UNREACHABLE();
}

}  // namespace impeller
