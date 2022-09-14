// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/render_pass_vk.h"
#include <array>

#include "fml/logging.h"
#include "impeller/renderer/backend/vulkan/surface_producer_vk.h"
#include "impeller/renderer/backend/vulkan/texture_vk.h"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {

static uint32_t color_flash = 0;

RenderPassVK::RenderPassVK(std::weak_ptr<const Context> context,
                           vk::Device device,
                           RenderTarget target,
                           vk::UniqueCommandBuffer command_buffer,
                           vk::UniqueRenderPass render_pass,
                           SurfaceProducerVK* surface_producer)
    : RenderPass(context, target),
      device_(device),
      command_buffer_(std::move(command_buffer)),
      render_pass_(std::move(render_pass)),
      surface_producer_(surface_producer) {
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
  auto res = command_buffer_->begin(begin_info);
  if (res != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to begin command buffer: " << vk::to_string(res);
    return false;
  }

  const auto& color0 = render_target.GetColorAttachments().at(0u);
  const auto& depth0 = render_target.GetDepthAttachment();
  const auto& stencil0 = render_target.GetStencilAttachment();

  auto& wrapped_texture = TextureVK::Cast(*color0.texture);
  FML_CHECK(wrapped_texture.IsWrapped());

  // maybe get the semaphores here.
  // signal the submission of command buffer here!.

  auto tex_info = wrapped_texture.GetTextureInfo()->wrapped_texture;
  vk::UniqueFramebuffer framebuffer = CreateFrameBuffer(tex_info);

  uint32_t frame_num = tex_info.frame_num;
  FML_LOG(ERROR) << __PRETTY_FUNCTION__ << ": frame_num: " << frame_num;

  // if (commands_.empty()) {
  //   FML_LOG(ERROR) << "NO COMMANDS TO ENCODE!!!";
  //   return const_cast<RenderPassVK*>(this)->EndCommandBuffer(frame_num);
  // }

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

  // layout transition?
  {
    auto pool = command_buffer_.getPool();
    vk::CommandBufferAllocateInfo alloc_info =
        vk::CommandBufferAllocateInfo()
            .setCommandPool(pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);
    auto cmd_buf_res = device_.allocateCommandBuffersUnique(alloc_info);
    if (cmd_buf_res.result != vk::Result::eSuccess) {
      VALIDATION_LOG << "Failed to allocate command buffer: "
                     << vk::to_string(cmd_buf_res.result);
      return false;
    }
    auto cmd_buf_one_time = std::move(cmd_buf_res.value[0]);

    vk::CommandBufferBeginInfo begin_info;
    auto res = cmd_buf_one_time->begin(begin_info);

    vk::ImageMemoryBarrier barrier =
        vk::ImageMemoryBarrier()
            .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setImage(tex_info.swapchain_image->GetImage())
            .setSubresourceRange(
                vk::ImageSubresourceRange()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1));
    cmd_buf_one_time->pipelineBarrier(
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, nullptr, nullptr,
        barrier);

    res = cmd_buf_one_time->end();
    if (res != vk::Result::eSuccess) {
      VALIDATION_LOG << "Failed to end command buffer: " << vk::to_string(res);
      return false;
    }

    surface_producer_->QueueCommandBuffer(frame_num,
                                          std::move(cmd_buf_one_time));
  }

  vk::ClearValue clear_value;
  float flash = abs(sin(color_flash++ / 120.f));
  clear_value.color =
      vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, flash, 1.0f});

  std::array<vk::ImageView, 1> fbo_attachments = {
      tex_info.swapchain_image->GetImageView(),
  };

  const auto& size = tex_info.swapchain_image->GetSize();

  // TODO destroy fbo after present.
  vk::FramebufferCreateInfo fb_create_info =
      vk::FramebufferCreateInfo()
          .setAttachments(fbo_attachments)
          .setRenderPass(*render_pass_)
          .setWidth(size.width)
          .setHeight(size.height)
          .setLayers(1);
  auto fb_res = device_.createFramebuffer(fb_create_info);
  if (fb_res.result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to create framebuffer: "
                   << vk::to_string(fb_res.result);
    return false;
  }

  FML_LOG(ERROR) << "rendering to swapchain image: "
                 << tex_info.swapchain_image->GetImage();

  vk::Rect2D render_area =
      vk::Rect2D()
          .setOffset(vk::Offset2D(0, 0))
          .setExtent(vk::Extent2D(size.width, size.height));
  auto rp_begin_info = vk::RenderPassBeginInfo()
                           .setRenderPass(*render_pass_)
                           .setFramebuffer(fb_res.value)
                           .setRenderArea(render_area)
                           .setClearValues(clear_value);

  command_buffer_->beginRenderPass(rp_begin_info, vk::SubpassContents::eInline);
  command_buffer_->endRenderPass();

  return const_cast<RenderPassVK*>(this)->EndCommandBuffer(frame_num);
}

bool RenderPassVK::EndCommandBuffer(uint32_t frame_num) {
  if (command_buffer_) {
    auto res = command_buffer_->end();
    if (res != vk::Result::eSuccess) {
      FML_LOG(ERROR) << "Failed to end command buffer: " << vk::to_string(res);
      return false;
    }

    surface_producer_->StashRP(frame_num, std::move(render_pass_));

    return surface_producer_->QueueCommandBuffer(frame_num,
                                                 std::move(command_buffer_));
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
