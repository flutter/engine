// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_pool_vk.h"

namespace impeller {

std::shared_ptr<CommandPoolVK> CommandPoolVK::Create(vk::Device device,
                                                     uint32_t queue_index) {
  vk::CommandPoolCreateInfo create_info;
  create_info.setQueueFamilyIndex(queue_index);

  auto res = device.createCommandPoolUnique(create_info);
  if (res.result != vk::Result::eSuccess) {
    FML_CHECK(false) << "Failed to create command pool: "
                     << vk::to_string(res.result);
    return nullptr;
  }

  return std::make_shared<CommandPoolVK>(device, std::move(res.value));
}

vk::CommandBuffer CommandPoolVK::CreateCommandBuffer() {
  std::scoped_lock<std::mutex> lock(pool_mutex_);

  vk::CommandBufferAllocateInfo allocate_info;
  allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
  allocate_info.setCommandBufferCount(1);
  allocate_info.setCommandPool(command_pool_.get());

  auto res = device_.allocateCommandBuffers(allocate_info);
  if (res.result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to allocate command buffer: "
                   << vk::to_string(res.result);
    return nullptr;
  }

  return res.value[0];
}

void CommandPoolVK::FreeCommandBuffers(
    const std::vector<vk::CommandBuffer>& buffers) {
  std::scoped_lock<std::mutex> lock(pool_mutex_);
  device_.freeCommandBuffers(command_pool_.get(), buffers);
}

CommandPoolVK::CommandPoolVK(vk::Device device,
                             vk::UniqueCommandPool command_pool)
    : device_(device), command_pool_(std::move(command_pool)) {}

CommandPoolVK::~CommandPoolVK() = default;

}  // namespace impeller
