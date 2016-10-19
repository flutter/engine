// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>
#include "flutter/vulkan/vulkan_backbuffer.h"
#include "flutter/vulkan/vulkan_proc_table.h"
#include "third_party/skia/include/gpu/vk/GrVkTypes.h"
#include "third_party/skia/src/gpu/vk/GrVkUtil.h"

namespace vulkan {

VulkanBackbuffer::VulkanBackbuffer(const VulkanProcTable& p_vk,
                                   const VulkanHandle<VkDevice>& device,
                                   const VulkanHandle<VkCommandPool>& pool)
    : vk(p_vk),
      device_(device),
      usage_command_buffer_(p_vk, device, pool),
      render_command_buffer_(p_vk, device, pool),
      valid_(false) {
  if (!usage_command_buffer_.IsValid() || !render_command_buffer_.IsValid()) {
    return;
  }

  if (!CreateSemaphores()) {
    return;
  }

  if (!CreateFences()) {
    return;
  }

  valid_ = true;
}

VulkanBackbuffer::~VulkanBackbuffer() {
  FTL_ALLOW_UNUSED_LOCAL(WaitFences());
}

bool VulkanBackbuffer::IsValid() const {
  return valid_;
}

bool VulkanBackbuffer::CreateSemaphores() {
  const VkSemaphoreCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };

  auto semaphore_collect = [this](VkSemaphore semaphore) {
    vk.destroySemaphore(device_, semaphore, nullptr);
  };

  for (size_t i = 0; i < semaphores_.size(); i++) {
    VkSemaphore semaphore = VK_NULL_HANDLE;

    if (vk.createSemaphore(device_, &create_info, nullptr, &semaphore) !=
        VK_SUCCESS) {
      return false;
    }

    semaphores_[i] = {semaphore, semaphore_collect};
  }

  return true;
}

bool VulkanBackbuffer::CreateFences() {
  const VkFenceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  auto fence_collect = [this](VkFence fence) {
    vk.destroyFence(device_, fence, nullptr);
  };

  for (size_t i = 0; i < use_fences_.size(); i++) {
    VkFence fence = VK_NULL_HANDLE;

    if (vk.createFence(device_, &create_info, nullptr, &fence) != VK_SUCCESS) {
      return false;
    }

    use_fences_[i] = {fence, fence_collect};
  }

  return true;
}

bool VulkanBackbuffer::WaitFences() {
  VkFence fences[use_fences_.size()];

  for (size_t i = 0; i < use_fences_.size(); i++) {
    fences[i] = use_fences_[i];
  }

  return vk.waitForFences(device_, static_cast<uint32_t>(use_fences_.size()),
                          fences, true,
                          std::numeric_limits<uint64_t>::max()) == VK_SUCCESS;
}

bool VulkanBackbuffer::ResetFences() {
  VkFence fences[use_fences_.size()];

  for (size_t i = 0; i < use_fences_.size(); i++) {
    fences[i] = use_fences_[i];
  }

  return vk.resetFences(device_, static_cast<uint32_t>(use_fences_.size()),
                        fences) == VK_SUCCESS;
}

const VulkanHandle<VkFence>& VulkanBackbuffer::UsageFence() const {
  return use_fences_[0];
}

const VulkanHandle<VkFence>& VulkanBackbuffer::RenderFence() const {
  return use_fences_[1];
}

const VulkanHandle<VkSemaphore>& VulkanBackbuffer::UsageSemaphore() const {
  return semaphores_[0];
}

const VulkanHandle<VkSemaphore>& VulkanBackbuffer::RenderSemaphore() const {
  return semaphores_[1];
}

VulkanCommandBuffer& VulkanBackbuffer::UsageCommandBuffer() {
  return usage_command_buffer_;
}

VulkanCommandBuffer& VulkanBackbuffer::RenderCommandBuffer() {
  return render_command_buffer_;
}

}  // namespace vulkan
