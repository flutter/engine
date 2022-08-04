#pragma once

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/backend/vulkan/swapchain_vk.h"
#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

// maybe call this framebuffers!!
class FramebufferVK {
 public:
  static std::shared_ptr<FramebufferVK> Create(
      vk::Device device,
      const RenderTarget& render_target,
      vk::RenderPass render_pass);

  explicit FramebufferVK(vk::UniqueFramebuffer framebuffer)
      : framebuffer_(std::move(framebuffer)) {}

  vk::UniqueFramebuffer framebuffer_;
};

}  // namespace impeller
