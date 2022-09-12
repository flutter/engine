// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/render_pass_vk.h"

#include "fml/logging.h"
#include "impeller/renderer/backend/vulkan/texture_vk.h"

namespace impeller {

RenderPassVK::RenderPassVK(std::weak_ptr<const Context> context,
                           vk::Device device,
                           RenderTarget target,
                           vk::CommandBuffer command_buffer,
                           vk::UniqueRenderPass render_pass)
    : RenderPass(context, target),
      device_(device),
      command_buffer_(command_buffer),
      render_pass_(std::move(render_pass)) {
  is_valid_ = true;
}

RenderPassVK::~RenderPassVK() = default;

bool RenderPassVK::IsValid() const {
  return is_valid_;
}

void RenderPassVK::OnSetLabel(std::string label) {
  label_ = std::move(label);
}

bool RenderPassVK::OnEncodeCommands(const Context& context) const {
  if (!IsValid()) {
    return false;
  }

  const auto& render_target = GetRenderTarget();
  if (!render_target.HasColorAttachment(0u)) {
    return false;
  }

  vk::CommandBufferBeginInfo begin_info;
  auto res = command_buffer_.begin(begin_info);
  if (res != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to begin command buffer: " << vk::to_string(res);
    return false;
  }

  if (commands_.empty()) {
    return EndCommandBuffer();
  }

  const auto& color0 = render_target.GetColorAttachments().at(0u);
  const auto& depth0 = render_target.GetDepthAttachment();
  const auto& stencil0 = render_target.GetStencilAttachment();

  auto& wrapped_texture = TextureVK::Cast(*color0.texture);
  FML_CHECK(wrapped_texture.IsWrapped());

  auto tex_info = wrapped_texture.GetTextureInfo()->wrapped_texture;
  vk::UniqueFramebuffer framebuffer = CreateFrameBuffer(tex_info);

  // swapchain framebuffer.
  for (const auto& command : commands_) {
    if (command.index_count == 0u) {
      continue;
    }
    if (command.instance_count == 0u) {
      continue;
    }

    const auto& pipeline_desc = command.pipeline->GetDescriptor();
  }

  return EndCommandBuffer();
}

bool RenderPassVK::EndCommandBuffer() const {
  if (command_buffer_) {
    auto res = command_buffer_.end();
    if (res != vk::Result::eSuccess) {
      FML_LOG(ERROR) << "Failed to end command buffer: " << vk::to_string(res);
      return false;
    }
    return true;
  }
  return false;
}

vk::UniqueFramebuffer RenderPassVK::CreateFrameBuffer(
    const WrappedTextureInfoVK& wrapped_texture_info) const {
  auto img_view = wrapped_texture_info.swapchain_image->GetImageView();
  auto size = wrapped_texture_info.swapchain_image->GetSize();
  vk::FramebufferCreateInfo fb_create_info = vk::FramebufferCreateInfo()
                                                 .setRenderPass(*render_pass_)
                                                 .setAttachmentCount(1)
                                                 .setPAttachments(&img_view)
                                                 .setWidth(size.width)
                                                 .setHeight(size.height)
                                                 .setLayers(1);
  auto res = device_.createFramebufferUnique(fb_create_info);
  FML_CHECK(res.result == vk::Result::eSuccess);
  return std::move(res.value);
}

}  // namespace impeller
