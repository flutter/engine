// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/render_pass_vk.h"
#include "fml/logging.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/render_target.h"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {

static vk::Extent2D GetExtent(const RenderTarget& rt) {
  return vk::Extent2D{static_cast<uint32_t>(rt.GetRenderTargetSize().width),
                      static_cast<uint32_t>(rt.GetRenderTargetSize().height)};
}

RenderPassVK::RenderPassVK(RenderTarget target,
                           vk::CommandBuffer command_buffer,
                           vk::UniqueRenderPass render_pass)
    : RenderPass(target),
      command_buffer_(command_buffer),
      render_pass_(std::move(render_pass)) {}

RenderPassVK::~RenderPassVK() = default;

bool RenderPassVK::IsValid() const {
  return true;
}

void RenderPassVK::OnSetLabel(std::string label) {
  // context_->SetDebugName(render_pass_->debugReportObjectType, label);
}

bool RenderPassVK::EncodeCommands(
    const std::shared_ptr<Allocator>& transients_allocator) const {
  vk::CommandBufferBeginInfo begin_info;
  auto begin_res = command_buffer_.begin(begin_info);
  if (begin_res != vk::Result::eSuccess) {
    FML_LOG(ERROR) << "Failed to begin command buffer: "
                   << vk::to_string(begin_res);
    return false;
  }

  vk::RenderPassBeginInfo render_pass_begin_info;
  render_pass_begin_info.setRenderPass(*render_pass_);
  // TODO (kaushikiska@)
  // renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

  render_pass_begin_info.renderArea.offset = vk::Offset2D{0, 0};
  render_pass_begin_info.renderArea.extent = GetExtent(render_target_);

  // commands still need to be encoded?!

  return true;
}

}  // namespace impeller
