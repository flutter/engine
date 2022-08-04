#pragma once

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/backend/vulkan/swapchain_vk.h"
#include "impeller/renderer/backend/vulkan/vk.h"

namespace impeller {

// maybe call this framebuffers!!
class FramebufferVK {
 public:
  static std::shared_ptr<FramebufferVK> Create(
      vk::Device device,
      std::shared_ptr<SwapchainVK> swapchain,
      vk::RenderPass render_pass);

  explicit FramebufferVK(std::vector<vk::UniqueFramebuffer> framebuffers);

  std::vector<vk::UniqueFramebuffer> framebuffers_;
};

}  // namespace impeller
