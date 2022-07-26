// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_buffer_vk.h"

#include "flutter/fml/logging.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

CommandBufferVK::CommandBufferVK(VULKAN_HPP_DEFAULT_DISPATCHER_TYPE& dispatch,
                                 vk::CommandBuffer buffer)
    : dispatch_(dispatch), buffer_(buffer) {}

CommandBufferVK::~CommandBufferVK() = default;

void CommandBufferVK::SetLabel(const std::string& label) const {
  FML_UNREACHABLE();
}

bool CommandBufferVK::IsValid() const {
  return buffer_;
}

bool CommandBufferVK::SubmitCommands(CompletionCallback callback) {
  auto end_result = buffer_.end(dispatch_);
  if (end_result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Unable to end the command buffer";
    return false;
  }

  FML_UNREACHABLE();
}

std::shared_ptr<RenderPass> CommandBufferVK::OnCreateRenderPass(
    RenderTarget target) const {
  vk::CommandBufferBeginInfo command_bufferbegin_info;
  command_bufferbegin_info.setFlags(vk::CommandBufferUsageFlags{
      VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
  });

  auto begin_result = buffer_.begin(command_bufferbegin_info, dispatch_);
  if (begin_result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Unable to begin the command buffer";
    return nullptr;
  }

  vk::UniqueRenderPass render_pass;

  vk::Rect2D render_area;
  render_area.setOffset({0, 0});
  render_area.setExtent({
      static_cast<uint32_t>(target.GetRenderTargetSize().width),
      static_cast<uint32_t>(target.GetRenderTargetSize().height),
  });

  vk::RenderPassBeginInfo render_pass_begin_info;
  render_pass_begin_info.setRenderPass(*render_pass);
  render_pass_begin_info.setRenderArea(render_area);

  // TODO (configure the render pass)

  FML_UNREACHABLE();
}

}  // namespace impeller
