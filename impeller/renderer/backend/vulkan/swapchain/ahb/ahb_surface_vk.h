// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_AHB_AHB_SURFACE_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_AHB_AHB_SURFACE_VK_H_

#include "impeller/renderer/backend/vulkan/android/ahb_texture_source_vk.h"
#include "impeller/renderer/backend/vulkan/swapchain/ahb/ahb_swapchain_vk.h"
#include "impeller/renderer/render_target.h"
#include "impeller/renderer/surface.h"

namespace impeller {

class AHBSurfaceVK final : public Surface,
                           public std::enable_shared_from_this<AHBSurfaceVK> {
 public:
  static std::shared_ptr<AHBSurfaceVK> WrapSwapchainImage(
      const std::shared_ptr<Context>& context,
      std::weak_ptr<AHBSwapchainVK> weak_swapchain,
      std::shared_ptr<AHBTextureSourceVK> swapchain_image);

  // |Surface|
  ~AHBSurfaceVK() override;

  AHBSurfaceVK(const AHBSurfaceVK&) = delete;

  AHBSurfaceVK& operator=(const AHBSurfaceVK&) = delete;

  // |Surface|
  bool Present() override;

  const std::shared_ptr<AHBTextureSourceVK>& GetTextureSource() const;

 private:
  std::weak_ptr<AHBSwapchainVK> weak_swapchain_;
  std::shared_ptr<AHBTextureSourceVK> swapchain_image_;

  AHBSurfaceVK(const RenderTarget& render_target,
               std::weak_ptr<AHBSwapchainVK> weak_swapchain,
               std::shared_ptr<AHBTextureSourceVK> texture);
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_AHB_AHB_SURFACE_VK_H_
