// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_buffer_vk.h"

#include "flutter/fml/logging.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

std::shared_ptr<CommandBufferVK> CommandBufferVK::Create(
    vk::Device device,
    vk::CommandPool command_pool) {
  vk::CommandBufferAllocateInfo allocate_info;
  allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
  allocate_info.setCommandBufferCount(1);
  allocate_info.setCommandPool(command_pool);

  auto res = device.allocateCommandBuffersUnique(allocate_info);
  if (res.result != vk::Result::eSuccess) {
    FML_CHECK(false) << "Failed to allocate command buffer: "
                     << vk::to_string(res.result);
    return nullptr;
  }

  vk::UniqueCommandBuffer cmd = std::move(res.value[0]);
  return std::make_shared<CommandBufferVK>(std::move(cmd));
}

CommandBufferVK::CommandBufferVK(vk::UniqueCommandBuffer cmd)
    : command_buffer_(std::move(cmd)) {}

CommandBufferVK::~CommandBufferVK() = default;

void CommandBufferVK::SetLabel(const std::string& label) const {
  FML_UNREACHABLE();
}

bool CommandBufferVK::IsValid() const {
  FML_UNREACHABLE();
}

bool CommandBufferVK::SubmitCommands(CompletionCallback callback) {
  FML_UNREACHABLE();
}

std::shared_ptr<RenderPass> CommandBufferVK::OnCreateRenderPass(
    RenderTarget target) const {
  FML_UNREACHABLE();
}

std::shared_ptr<BlitPass> CommandBufferVK::OnCreateBlitPass() const {
  FML_UNREACHABLE();
}

}  // namespace impeller
