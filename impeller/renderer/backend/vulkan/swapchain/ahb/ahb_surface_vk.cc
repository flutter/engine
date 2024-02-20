// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/swapchain/ahb/ahb_surface_vk.h"

#include "impeller/core/texture.h"
#include "impeller/renderer/backend/vulkan/texture_vk.h"

namespace impeller {

std::shared_ptr<AHBSurfaceVK> AHBSurfaceVK::WrapSwapchainImage(
    const std::shared_ptr<Context>& context,
    std::weak_ptr<AHBSwapchainVK> weak_swapchain,
    std::shared_ptr<AHBTextureSourceVK> swapchain_image) {
  if (!swapchain_image || !swapchain_image->IsValid() || !context) {
    VALIDATION_LOG << "Invalid swapchain image to wrap.";
    return nullptr;
  }

  TextureDescriptor msaa_tex_desc;
  msaa_tex_desc.storage_mode = StorageMode::kDeviceTransient;
  msaa_tex_desc.type = TextureType::kTexture2DMultisample;
  msaa_tex_desc.sample_count = SampleCount::kCount4;
  msaa_tex_desc.format = swapchain_image->GetTextureDescriptor().format;
  msaa_tex_desc.size = swapchain_image->GetTextureDescriptor().size;
  msaa_tex_desc.usage = static_cast<uint64_t>(TextureUsage::kRenderTarget);
  msaa_tex_desc.compression_type = CompressionType::kLossy;

  auto msaa_tex = context->GetResourceAllocator()->CreateTexture(msaa_tex_desc);
  auto resolve_tex = std::make_shared<TextureVK>(context, swapchain_image);

  if (!msaa_tex || !resolve_tex) {
    return nullptr;
  }

  ColorAttachment color0;
  color0.clear_color = Color::DarkSlateGray();
  color0.load_action = LoadAction::kClear;
  color0.texture = msaa_tex;
  color0.resolve_texture = resolve_tex;
  color0.store_action = StoreAction::kMultisampleResolve;

  RenderTarget render_target;
  render_target.SetColorAttachment(color0, 0u);

  auto surface = std::shared_ptr<AHBSurfaceVK>(new AHBSurfaceVK(
      render_target,              //
      std::move(weak_swapchain),  //
      std::move(swapchain_image)  //
      ));
  if (!surface->IsValid()) {
    VALIDATION_LOG << "Surface for wrapped swapchain image was invalid.";
    return nullptr;
  }
  return surface;
}

AHBSurfaceVK::AHBSurfaceVK(const RenderTarget& render_target,
                           std::weak_ptr<AHBSwapchainVK> weak_swapchain,
                           std::shared_ptr<AHBTextureSourceVK> swapchain_image)
    : Surface(render_target),
      weak_swapchain_(std::move(weak_swapchain)),
      swapchain_image_(std::move(swapchain_image)) {}

AHBSurfaceVK::~AHBSurfaceVK() = default;

// |Surface|
bool AHBSurfaceVK::Present() {
  if (!swapchain_image_) {
    VALIDATION_LOG << "Invalid swapchain image.";
    return false;
  }
  auto swapchain = weak_swapchain_.lock();
  if (!swapchain) {
    VALIDATION_LOG << "Swapchain died before presentation.";
    return false;
  }
  return swapchain->PresentSurface(shared_from_this());
}

const std::shared_ptr<AHBTextureSourceVK>& AHBSurfaceVK::GetTextureSource()
    const {
  return swapchain_image_;
}

}  // namespace impeller
