// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>
#include <vector>
#include "flutter/vulkan/vulkan_device.h"
#include "flutter/vulkan/vulkan_proc_table.h"
#include "flutter/vulkan/vulkan_surface.h"
#include "third_party/skia/include/gpu/vk/GrVkBackendContext.h"

namespace vulkan {

static uint32_t FindGraphicsQueueIndex(
    const std::vector<VkQueueFamilyProperties>& properties) {
  for (uint32_t i = 0, count = static_cast<uint32_t>(properties.size());
       i < count; i++) {
    if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      return i;
    }
  }
  return std::numeric_limits<uint32_t>::max();
}

VulkanDevice::VulkanDevice(const VulkanProcTable& p_vk,
                           VulkanHandle<VkPhysicalDevice> physical_device)
    : vk(p_vk),
      physical_device_(std::move(physical_device)),
      graphics_queue_index_(std::numeric_limits<uint32_t>::max()),
      valid_(false) {
  if (!physical_device_) {
    return;
  }

  graphics_queue_index_ = FindGraphicsQueueIndex(GetQueueFamilyProperties());

  if (graphics_queue_index_ == std::numeric_limits<uint32_t>::max()) {
    return;
  }

  float priorities[1] = {1.0f};

  const VkDeviceQueueCreateInfo queue_create = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = graphics_queue_index_,
      .queueCount = 1,
      .pQueuePriorities = priorities,
  };

  const char* extensions[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  const VkDeviceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_create,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = sizeof(extensions) / sizeof(const char*),
      .ppEnabledExtensionNames = extensions,
      .pEnabledFeatures = nullptr,
  };

  VkDevice device = VK_NULL_HANDLE;

  if (vk.createDevice(physical_device_, &create_info, nullptr, &device) !=
      VK_SUCCESS) {
    return;
  }

  device_ = {device,
             [this](VkDevice device) { vk.destroyDevice(device, nullptr); }};

  VkQueue queue = VK_NULL_HANDLE;

  vk.getDeviceQueue(device_, graphics_queue_index_, 0, &queue);

  if (queue == VK_NULL_HANDLE) {
    return;
  }

  queue_ = queue;

  const VkCommandPoolCreateInfo command_pool_create_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = 0,
  };

  VkCommandPool command_pool = VK_NULL_HANDLE;
  if (vk.createCommandPool(device_, &command_pool_create_info, nullptr,
                           &command_pool) != VK_SUCCESS) {
    return;
  }

  command_pool_ = {command_pool, [this](VkCommandPool pool) {
                     vk.destroyCommandPool(device_, pool, nullptr);
                   }};

  valid_ = true;
}

VulkanDevice::~VulkanDevice() {
  FTL_ALLOW_UNUSED_LOCAL(WaitIdle());
}

bool VulkanDevice::IsValid() const {
  return valid_;
}

bool VulkanDevice::WaitIdle() const {
  return vk.deviceWaitIdle(device_) == VK_SUCCESS;
}

const VulkanHandle<VkDevice>& VulkanDevice::Handle() const {
  return device_;
}

const VulkanHandle<VkPhysicalDevice>& VulkanDevice::PhysicalDeviceHandle()
    const {
  return physical_device_;
}

const VulkanHandle<VkQueue>& VulkanDevice::QueueHandle() const {
  return queue_;
}

const VulkanHandle<VkCommandPool>& VulkanDevice::CommandPool() const {
  return command_pool_;
}

uint32_t VulkanDevice::GraphicsQueueIndex() const {
  return graphics_queue_index_;
}

bool VulkanDevice::GetSurfaceCapabilities(
    const VulkanSurface& surface,
    VkSurfaceCapabilitiesKHR* capabilities) const {
  if (!surface.IsValid()) {
    return false;
  }

  return vk.getPhysicalDeviceSurfaceCapabilitiesKHR(
             physical_device_, surface.Handle(), capabilities) == VK_SUCCESS;
}

bool VulkanDevice::GetPhysicalDeviceFeatures(
    VkPhysicalDeviceFeatures* features) const {
  if (features == nullptr || !physical_device_) {
    return false;
  }
  vk.getPhysicalDeviceFeatures(physical_device_, features);
  return true;
}

bool VulkanDevice::GetPhysicalDeviceFeaturesSkia(uint32_t* sk_features) const {
  if (sk_features == nullptr) {
    return false;
  }

  VkPhysicalDeviceFeatures features = {0};
  if (!GetPhysicalDeviceFeatures(&features)) {
    return false;
  }

  uint32_t flags = 0;

  if (features.geometryShader) {
    flags |= kGeometryShader_GrVkFeatureFlag;
  }
  if (features.dualSrcBlend) {
    flags |= kDualSrcBlend_GrVkFeatureFlag;
  }
  if (features.sampleRateShading) {
    flags |= kSampleRateShading_GrVkFeatureFlag;
  }

  *sk_features = flags;
  return true;
}

std::vector<VkQueueFamilyProperties> VulkanDevice::GetQueueFamilyProperties()
    const {
  uint32_t count = 0;

  vk.getPhysicalDeviceQueueFamilyProperties(physical_device_, &count, nullptr);

  std::vector<VkQueueFamilyProperties> properties;
  properties.resize(count, {0});

  vk.getPhysicalDeviceQueueFamilyProperties(physical_device_, &count,
                                            properties.data());

  return properties;
}

bool VulkanDevice::ChooseSurfaceFormat(const VulkanSurface& surface,
                                       VkSurfaceFormatKHR* format) const {
  if (!surface.IsValid() || format == nullptr) {
    return false;
  }

  uint32_t format_count = 0;
  if (vk.getPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface.Handle(),
                                            &format_count,
                                            nullptr) != VK_SUCCESS) {
    return false;
  }

  if (format_count == 0) {
    return false;
  }

  VkSurfaceFormatKHR formats[format_count];
  if (vk.getPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface.Handle(),
                                            &format_count,
                                            formats) != VK_SUCCESS) {
    return false;
  }

  for (uint32_t i = 0; i < format_count; i++) {
    if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM) {
      *format = formats[i];
      return true;
    }
  }

  return false;
}

bool VulkanDevice::ChoosePresentMode(const VulkanSurface& surface,
                                     VkPresentModeKHR* present_mode) const {
  if (!surface.IsValid() || present_mode == nullptr) {
    return false;
  }

  uint32_t modes_count = 0;

  if (vk.getPhysicalDeviceSurfacePresentModesKHR(physical_device_,
                                                 surface.Handle(), &modes_count,
                                                 nullptr) != VK_SUCCESS) {
    return false;
  }

  if (modes_count == 0) {
    return false;
  }

  VkPresentModeKHR modes[modes_count];

  if (vk.getPhysicalDeviceSurfacePresentModesKHR(physical_device_,
                                                 surface.Handle(), &modes_count,
                                                 modes) != VK_SUCCESS) {
    return false;
  }

  // Prefer the mailbox mode if available.
  for (uint32_t i = 0; i < modes_count; i++) {
    if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      *present_mode = modes[i];
      return true;
    }
  }

  // Fallback to FIFO mode.
  *present_mode = VK_PRESENT_MODE_FIFO_KHR;
  return true;
}

bool VulkanDevice::QueueSubmit(
    std::vector<VkPipelineStageFlags> wait_dest_pipeline_stages,
    const std::vector<VkSemaphore>& wait_semaphores,
    const std::vector<VkSemaphore>& signal_semaphores,
    const std::vector<VkCommandBuffer>& command_buffers,
    const VulkanHandle<VkFence>& fence) const {
  if (wait_semaphores.size() != wait_dest_pipeline_stages.size()) {
    return false;
  }

  const VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size()),
      .pWaitSemaphores = wait_semaphores.data(),
      .pWaitDstStageMask = wait_dest_pipeline_stages.data(),
      .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
      .pCommandBuffers = command_buffers.data(),
      .signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size()),
      .pSignalSemaphores = signal_semaphores.data(),
  };

  if (vk.queueSubmit(queue_, 1, &submit_info, fence) != VK_SUCCESS) {
    return false;
  }

  return true;
}

}  // namespace vulkan
