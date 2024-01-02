// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/render_pass_vk.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>
#include <vector>

#include "flutter/fml/trace_event.h"
#include "impeller/base/validation.h"
#include "impeller/core/formats.h"
#include "impeller/renderer/backend/vulkan/barrier_vk.h"
#include "impeller/renderer/backend/vulkan/binding_helpers_vk.h"
#include "impeller/renderer/backend/vulkan/command_buffer_vk.h"
#include "impeller/renderer/backend/vulkan/command_encoder_vk.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/device_buffer_vk.h"
#include "impeller/renderer/backend/vulkan/formats_vk.h"
#include "impeller/renderer/backend/vulkan/pipeline_vk.h"
#include "impeller/renderer/backend/vulkan/render_pass_builder_vk.h"
#include "impeller/renderer/backend/vulkan/shared_object_vk.h"
#include "impeller/renderer/backend/vulkan/texture_vk.h"
#include "impeller/renderer/command.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_to_string.hpp"

namespace impeller {

SharedHandleVK<vk::RenderPass> RenderPassVK::CreateVKRenderPass(
    const ContextVK& context,
    const std::shared_ptr<CommandBufferVK>& command_buffer,
    size_t subpass_count) const {
  BarrierVK barrier;
  barrier.new_layout = vk::ImageLayout::eGeneral;
  barrier.cmd_buffer = command_buffer->GetEncoder()->GetCommandBuffer();
  barrier.src_access = vk::AccessFlagBits::eShaderRead;
  barrier.src_stage = vk::PipelineStageFlagBits::eFragmentShader;
  barrier.dst_access = vk::AccessFlagBits::eColorAttachmentWrite |
                       vk::AccessFlagBits::eTransferWrite;
  barrier.dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                      vk::PipelineStageFlagBits::eTransfer;

  RenderPassBuilderVK builder;

  builder.SetSubpassCount(subpass_count);

  for (const auto& [bind_point, color] : render_target_.GetColorAttachments()) {
    builder.SetColorAttachment(
        bind_point,                                          //
        color.texture->GetTextureDescriptor().format,        //
        color.texture->GetTextureDescriptor().sample_count,  //
        color.load_action,                                   //
        color.store_action                                   //
    );
    TextureVK::Cast(*color.texture).SetLayout(barrier);
    if (color.resolve_texture) {
      TextureVK::Cast(*color.resolve_texture).SetLayout(barrier);
    }
  }

  if (auto depth = render_target_.GetDepthAttachment(); depth.has_value()) {
    builder.SetDepthStencilAttachment(
        depth->texture->GetTextureDescriptor().format,        //
        depth->texture->GetTextureDescriptor().sample_count,  //
        depth->load_action,                                   //
        depth->store_action                                   //
    );
    TextureVK::Cast(*depth->texture).SetLayout(barrier);
  }

  if (auto stencil = render_target_.GetStencilAttachment();
      stencil.has_value()) {
    builder.SetStencilAttachment(
        stencil->texture->GetTextureDescriptor().format,        //
        stencil->texture->GetTextureDescriptor().sample_count,  //
        stencil->load_action,                                   //
        stencil->store_action                                   //
    );
    TextureVK::Cast(*stencil->texture).SetLayout(barrier);
  }

  auto pass = builder.Build(context.GetDevice());

  if (!pass) {
    VALIDATION_LOG << "Failed to create render pass for framebuffer.";
    return {};
  }

  context.SetDebugName(pass.get(), debug_label_.c_str());

  return MakeSharedVK(std::move(pass));
}

RenderPassVK::RenderPassVK(const std::shared_ptr<const Context>& context,
                           const RenderTarget& target,
                           std::weak_ptr<CommandBufferVK> command_buffer)
    : RenderPass(context, target), command_buffer_(std::move(command_buffer)) {
  is_valid_ = true;
}

RenderPassVK::~RenderPassVK() = default;

bool RenderPassVK::IsValid() const {
  return is_valid_;
}

void RenderPassVK::OnSetLabel(std::string label) {
  debug_label_ = std::move(label);
}

static vk::ClearColorValue VKClearValueFromColor(Color color) {
  vk::ClearColorValue value;
  value.setFloat32(
      std::array<float, 4>{color.red, color.green, color.blue, color.alpha});
  return value;
}

static vk::ClearDepthStencilValue VKClearValueFromDepthStencil(uint32_t stencil,
                                                               Scalar depth) {
  vk::ClearDepthStencilValue value;
  value.depth = depth;
  value.stencil = stencil;
  return value;
}

static std::vector<vk::ClearValue> GetVKClearValues(
    const RenderTarget& target) {
  std::vector<vk::ClearValue> clears;

  for (const auto& [_, color] : target.GetColorAttachments()) {
    clears.emplace_back(VKClearValueFromColor(color.clear_color));
    if (color.resolve_texture) {
      clears.emplace_back(VKClearValueFromColor(color.clear_color));
    }
  }

  const auto& depth = target.GetDepthAttachment();
  const auto& stencil = target.GetStencilAttachment();

  if (depth.has_value()) {
    clears.emplace_back(VKClearValueFromDepthStencil(
        stencil ? stencil->clear_stencil : 0u, depth->clear_depth));
  }

  if (stencil.has_value()) {
    clears.emplace_back(VKClearValueFromDepthStencil(
        stencil->clear_stencil, depth ? depth->clear_depth : 0.0f));
  }

  return clears;
}

SharedHandleVK<vk::Framebuffer> RenderPassVK::CreateVKFramebuffer(
    const ContextVK& context,
    const vk::RenderPass& pass) const {
  vk::FramebufferCreateInfo fb_info;

  fb_info.renderPass = pass;

  const auto target_size = render_target_.GetRenderTargetSize();
  fb_info.width = target_size.width;
  fb_info.height = target_size.height;
  fb_info.layers = 1u;

  std::vector<vk::ImageView> attachments;

  // This bit must be consistent to ensure compatibility with the pass created
  // earlier. Follow this order: Color attachments, then depth, then stencil.
  for (const auto& [_, color] : render_target_.GetColorAttachments()) {
    // The bind point doesn't matter here since that information is present in
    // the render pass.
    attachments.emplace_back(TextureVK::Cast(*color.texture).GetImageView());
    if (color.resolve_texture) {
      attachments.emplace_back(
          TextureVK::Cast(*color.resolve_texture).GetImageView());
    }
  }
  if (auto depth = render_target_.GetDepthAttachment(); depth.has_value()) {
    attachments.emplace_back(TextureVK::Cast(*depth->texture).GetImageView());
  }
  if (auto stencil = render_target_.GetStencilAttachment();
      stencil.has_value()) {
    attachments.emplace_back(TextureVK::Cast(*stencil->texture).GetImageView());
  }

  fb_info.setAttachments(attachments);

  auto [result, framebuffer] =
      context.GetDevice().createFramebufferUnique(fb_info);

  if (result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Could not create framebuffer: " << vk::to_string(result);
    return {};
  }

  return MakeSharedVK(std::move(framebuffer));
}

static bool UpdateBindingLayouts(const Bindings& bindings,
                                 const vk::CommandBuffer& buffer) {
  // All previous writes via a render or blit pass must be done before another
  // shader attempts to read the resource.
  BarrierVK barrier;
  barrier.cmd_buffer = buffer;
  barrier.src_access = vk::AccessFlagBits::eColorAttachmentWrite |
                       vk::AccessFlagBits::eTransferWrite;
  barrier.src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                      vk::PipelineStageFlagBits::eTransfer;
  barrier.dst_access = vk::AccessFlagBits::eShaderRead;
  barrier.dst_stage = vk::PipelineStageFlagBits::eFragmentShader;

  barrier.new_layout = vk::ImageLayout::eShaderReadOnlyOptimal;

  for (const TextureAndSampler& data : bindings.sampled_images) {
    if (!TextureVK::Cast(*data.texture.resource).SetLayout(barrier)) {
      return false;
    }
  }
  return true;
}

static bool UpdateBindingLayouts(const Command& command,
                                 const vk::CommandBuffer& buffer) {
  return UpdateBindingLayouts(command.vertex_bindings, buffer) &&
         UpdateBindingLayouts(command.fragment_bindings, buffer);
}

static bool UpdateBindingLayouts(const std::vector<Command>& commands,
                                 const vk::CommandBuffer& buffer) {
  for (const Command& command : commands) {
    if (!UpdateBindingLayouts(command, buffer)) {
      return false;
    }
  }
  return true;
}

static void SetViewportAndScissor(const Command& command,
                                  const vk::CommandBuffer& cmd_buffer,
                                  PassBindingsCache& cmd_buffer_cache,
                                  const ISize& target_size) {
  // Set the viewport.
  const auto& vp = command.viewport.value_or<Viewport>(
      {.rect = Rect::MakeSize(target_size)});
  vk::Viewport viewport = vk::Viewport()
                              .setWidth(vp.rect.GetWidth())
                              .setHeight(-vp.rect.GetHeight())
                              .setY(vp.rect.GetHeight())
                              .setMinDepth(0.0f)
                              .setMaxDepth(1.0f);
  cmd_buffer_cache.SetViewport(cmd_buffer, 0, 1, &viewport);

  // Set the scissor rect.
  const auto& sc = command.scissor.value_or(IRect::MakeSize(target_size));
  vk::Rect2D scissor =
      vk::Rect2D()
          .setOffset(vk::Offset2D(sc.GetX(), sc.GetY()))
          .setExtent(vk::Extent2D(sc.GetWidth(), sc.GetHeight()));
  cmd_buffer_cache.SetScissor(cmd_buffer, 0, 1, &scissor);
}

static bool EncodeCommand(const Context& context,
                          const Command& command,
                          CommandEncoderVK& encoder,
                          PassBindingsCache& command_buffer_cache,
                          const ISize& target_size,
                          const vk::DescriptorSet vk_desc_set,
                          SubpassCursorVK subpass_cursor) {
#ifdef IMPELLER_DEBUG
  fml::ScopedCleanupClosure pop_marker(
      [&encoder]() { encoder.PopDebugGroup(); });
  if (!command.label.empty()) {
    encoder.PushDebugGroup(command.label.c_str());
  } else {
    pop_marker.Release();
  }
#endif  // IMPELLER_DEBUG

  const auto& cmd_buffer = encoder.GetCommandBuffer();
  const auto& pipeline_vk = PipelineVK::Cast(*command.pipeline);

  encoder.GetCommandBuffer().bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,  // bind point
      pipeline_vk.GetPipelineLayout(),   // layout
      0,                                 // first set
      {vk::DescriptorSet{vk_desc_set}},  // sets
      nullptr                            // offsets
  );

  command_buffer_cache.BindPipeline(cmd_buffer,
                                    vk::PipelineBindPoint::eGraphics,
                                    pipeline_vk.GetPipeline(subpass_cursor));

  // Set the viewport and scissors.
  SetViewportAndScissor(command, cmd_buffer, command_buffer_cache, target_size);

  // Set the stencil reference.
  command_buffer_cache.SetStencilReference(
      cmd_buffer, vk::StencilFaceFlagBits::eVkStencilFrontAndBack,
      command.stencil_reference);

  // Configure vertex and index and buffers for binding.
  auto& vertex_buffer_view = command.vertex_buffer.vertex_buffer;

  if (!vertex_buffer_view) {
    return false;
  }

  auto& allocator = *context.GetResourceAllocator();
  auto vertex_buffer = vertex_buffer_view.buffer->GetDeviceBuffer();

  if (!vertex_buffer) {
    VALIDATION_LOG << "Failed to acquire device buffer"
                   << " for vertex buffer view";
    return false;
  }

  if (!encoder.Track(vertex_buffer)) {
    return false;
  }

  // Bind the vertex buffer.
  auto vertex_buffer_handle = DeviceBufferVK::Cast(*vertex_buffer).GetBuffer();
  vk::Buffer vertex_buffers[] = {vertex_buffer_handle};
  vk::DeviceSize vertex_buffer_offsets[] = {vertex_buffer_view.range.offset};
  cmd_buffer.bindVertexBuffers(0u, 1u, vertex_buffers, vertex_buffer_offsets);

  if (command.vertex_buffer.index_type != IndexType::kNone) {
    // Bind the index buffer.
    auto index_buffer_view = command.vertex_buffer.index_buffer;
    if (!index_buffer_view) {
      return false;
    }

    auto index_buffer = index_buffer_view.buffer->GetDeviceBuffer();
    if (!index_buffer) {
      VALIDATION_LOG << "Failed to acquire device buffer"
                     << " for index buffer view";
      return false;
    }

    if (!encoder.Track(index_buffer)) {
      return false;
    }

    auto index_buffer_handle = DeviceBufferVK::Cast(*index_buffer).GetBuffer();
    cmd_buffer.bindIndexBuffer(index_buffer_handle,
                               index_buffer_view.range.offset,
                               ToVKIndexType(command.vertex_buffer.index_type));

    // Engage!
    cmd_buffer.drawIndexed(command.vertex_buffer.vertex_count,  // index count
                           command.instance_count,  // instance count
                           0u,                      // first index
                           command.base_vertex,     // vertex offset
                           0u                       // first instance
    );
  } else {
    cmd_buffer.draw(command.vertex_buffer.vertex_count,  // vertex count
                    command.instance_count,              // instance count
                    command.base_vertex,                 // vertex offset
                    0u                                   // first instance
    );
  }
  return true;
}

static size_t CountSubpassesForCommandStream(
    const std::vector<Command>& commands) {
  if (commands.empty()) {
    return 1u;
  }
  auto commands_with_input_attachments =
      std::accumulate(commands.cbegin(), commands.cend(), 0u,
                      [](size_t count, const auto& command) {
                        return count + (command.pipeline->GetDescriptor()
                                                .GetVertexDescriptor()
                                                ->UsesInputAttacments()
                                            ? 1u
                                            : 0u);
                      });
  // If there are no commands with input attachments use, then we only need one
  // lone subpass. This should be the most common case.
  if (commands_with_input_attachments == 0) {
    return 1u;
  }
  const bool first_command_uses_input_attachments =
      commands.cbegin()
          ->pipeline->GetDescriptor()
          .GetVertexDescriptor()
          ->UsesInputAttacments();
  return first_command_uses_input_attachments
             ? commands_with_input_attachments
             : commands_with_input_attachments + 1u;
}

static bool ShouldAdvanceSubpass(const Command& command, size_t command_index) {
  const bool command_uses_input_attachments = command.pipeline->GetDescriptor()
                                                  .GetVertexDescriptor()
                                                  ->UsesInputAttacments();

  if (!command_uses_input_attachments) {
    return false;
  }

  if (command_index == 0 && command_uses_input_attachments) {
    return false;
  }

  return true;
}

static void PreloadSubpassPipelineForCommandStream(
    const std::vector<Command>& commands,
    size_t subpass_count) {
  if (subpass_count <= 1u) {
    return;
  }
  size_t subpass_index = 0u;
  for (size_t command_index = 0; command_index < commands.size();
       command_index++) {
    const auto& command = commands[command_index];
    if (ShouldAdvanceSubpass(command, command_index)) {
      subpass_index++;
    }
    PipelineVK::Cast(*command.pipeline)
        .PreloadPipeline(SubpassCursorVK{subpass_index, subpass_count});
  }
}

bool RenderPassVK::OnEncodeCommands(const Context& context) const {
  TRACE_EVENT0("impeller", "RenderPassVK::OnEncodeCommands");
  if (!IsValid()) {
    return false;
  }

  const auto& vk_context = ContextVK::Cast(context);

  auto command_buffer = command_buffer_.lock();
  if (!command_buffer) {
    VALIDATION_LOG << "Command buffer died before commands could be encoded.";
    return false;
  }
  auto encoder = command_buffer->GetEncoder();
  if (!encoder) {
    return false;
  }

  fml::ScopedCleanupClosure pop_marker(
      [&encoder]() { encoder->PopDebugGroup(); });
  if (!debug_label_.empty()) {
    encoder->PushDebugGroup(debug_label_.c_str());
  } else {
    pop_marker.Release();
  }

  auto cmd_buffer = encoder->GetCommandBuffer();

  if (!UpdateBindingLayouts(commands_, cmd_buffer)) {
    return false;
  }

  render_target_.IterateAllAttachments(
      [&encoder](const auto& attachment) -> bool {
        encoder->Track(attachment.texture);
        encoder->Track(attachment.resolve_texture);
        return true;
      });

  const auto& target_size = render_target_.GetRenderTargetSize();

  SubpassCursorVK subpass_cursor;
  subpass_cursor.count = CountSubpassesForCommandStream(commands_);

  PreloadSubpassPipelineForCommandStream(commands_, subpass_cursor.count);

  auto render_pass =
      CreateVKRenderPass(vk_context, command_buffer, subpass_cursor.count);
  if (!render_pass) {
    VALIDATION_LOG << "Could not create renderpass.";
    return false;
  }

  auto framebuffer = CreateVKFramebuffer(vk_context, *render_pass);
  if (!framebuffer) {
    VALIDATION_LOG << "Could not create framebuffer.";
    return false;
  }

  if (!encoder->Track(framebuffer) || !encoder->Track(render_pass)) {
    return false;
  }

  auto clear_values = GetVKClearValues(render_target_);

  vk::RenderPassBeginInfo pass_info;
  pass_info.renderPass = *render_pass;
  pass_info.framebuffer = *framebuffer;
  pass_info.renderArea.extent.width = static_cast<uint32_t>(target_size.width);
  pass_info.renderArea.extent.height =
      static_cast<uint32_t>(target_size.height);
  pass_info.setClearValues(clear_values);

  // Only attachment at index 0 may be used as a subpass input at this time.
  const auto& color_attachment0 =
      render_target_.GetColorAttachments().at(0u).texture;
  auto desc_sets_result = AllocateAndBindDescriptorSets(
      vk_context, encoder, commands_, color_attachment0);
  if (!desc_sets_result.ok()) {
    return false;
  }
  auto desc_sets = desc_sets_result.value();

  {
    TRACE_EVENT0("impeller", "EncodeRenderPassCommands");
    cmd_buffer.beginRenderPass(pass_info, vk::SubpassContents::eInline);

    fml::ScopedCleanupClosure end_render_pass(
        [cmd_buffer]() { cmd_buffer.endRenderPass(); });

    size_t command_index = 0u;
    for (const auto& command : commands_) {
      if (ShouldAdvanceSubpass(command, command_index)) {
        subpass_cursor.index++;
        encoder->GetCommandBuffer().nextSubpass(vk::SubpassContents::eInline);
      }
      FML_DCHECK(subpass_cursor.IsValid());
      FML_DCHECK(PipelineVK::Cast(*command.pipeline)
                     .HasPreloadedPipeline(subpass_cursor))
          << "Insufficient subpass pipeline preloading. Functionally correct "
             "but misses using available concurrency.";
      if (!EncodeCommand(context,                   //
                         command,                   //
                         *encoder,                  //
                         pass_bindings_cache_,      //
                         target_size,               //
                         desc_sets[command_index],  //
                         subpass_cursor             //
                         )) {
        return false;
      }
      command_index += 1;
    }
    FML_DCHECK(subpass_cursor.IsValid());
    FML_DCHECK(subpass_cursor.IsFinalSubpass());
  }

  return true;
}

}  // namespace impeller
