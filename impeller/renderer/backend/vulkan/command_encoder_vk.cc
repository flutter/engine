// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_encoder_vk.h"

#include "flutter/fml/closure.h"
#include "flutter/fml/trace_event.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"

namespace impeller {

CommandEncoderVK::CommandEncoderVK(vk::Device device,
                                   vk::Queue queue,
                                   vk::CommandPool pool)
    : desc_pool_(device) {
  vk::CommandBufferAllocateInfo alloc_info;
  alloc_info.commandPool = pool;
  alloc_info.commandBufferCount = 1u;
  alloc_info.level = vk::CommandBufferLevel::ePrimary;
  auto [result, buffers] = device.allocateCommandBuffersUnique(alloc_info);
  if (result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Could not create command buffer.";
    return;
  }
  vk::CommandBufferBeginInfo begin_info;
  begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  if (buffers[0]->begin(begin_info) != vk::Result::eSuccess) {
    VALIDATION_LOG << "Could not begin command buffer.";
    return;
  }
  device_ = device;
  queue_ = queue;
  command_buffer_ = std::move(buffers[0]);
  is_valid_ = true;
}

CommandEncoderVK::~CommandEncoderVK() = default;

bool CommandEncoderVK::IsValid() const {
  return is_valid_;
}

bool CommandEncoderVK::Submit() {
  if (!IsValid()) {
    return false;
  }

  // Success or failure, you only get to submit once.
  fml::ScopedCleanupClosure reset([&]() { Reset(); });

  if (command_buffer_->end() != vk::Result::eSuccess) {
    return false;
  }
  auto [fence_result, fence] = device_.createFenceUnique({});
  if (fence_result != vk::Result::eSuccess) {
    return false;
  }
  vk::SubmitInfo submit_info;
  submit_info.setCommandBuffers(*command_buffer_);
  if (queue_.submit(submit_info, *fence) != vk::Result::eSuccess) {
    return false;
  }

  {
    TRACE_EVENT0("impeller", "WaitForCompletion");
    if (device_.waitForFences(
            *fence,                               // fences
            true,                                 // wait all
            std::numeric_limits<uint64_t>::max()  // timeout (ns)
            ) != vk::Result::eSuccess) {
      return false;
    }
  }

  return true;
}

const vk::CommandBuffer& CommandEncoderVK::GetCommandBuffer() const {
  return *command_buffer_;
}

void CommandEncoderVK::Reset() {
  command_buffer_.reset();

  tracked_objects_.clear();
  tracked_buffers_.clear();
  tracked_textures_.clear();

  queue_ = nullptr;
  device_ = nullptr;
  is_valid_ = false;
}

bool CommandEncoderVK::Track(std::shared_ptr<SharedObjectVK> object) {
  tracked_objects_.insert(std::move(object));
  return true;
}

bool CommandEncoderVK::Track(std::shared_ptr<const DeviceBuffer> buffer) {
  tracked_buffers_.insert(std::move(buffer));
  return true;
}

bool CommandEncoderVK::Track(std::shared_ptr<const Texture> texture) {
  tracked_textures_.insert(std::move(texture));
  return true;
}

void CommandEncoderVK::PushDebugGroup(const char* label) const {
  if (!vk::HasValidationLayers() || !command_buffer_) {
    return;
  }
  vk::DebugUtilsLabelEXT label_info;
  label_info.pLabelName = label;
  command_buffer_->beginDebugUtilsLabelEXT(label_info);
}

void CommandEncoderVK::PopDebugGroup() const {
  if (!vk::HasValidationLayers() || !command_buffer_) {
    return;
  }
  command_buffer_->endDebugUtilsLabelEXT();
}

std::optional<vk::DescriptorSet> CommandEncoderVK::AllocateDescriptorSet(
    const vk::DescriptorSetLayout& layout) {
  return desc_pool_.AllocateDescriptorSet(layout);
}

}  // namespace impeller
