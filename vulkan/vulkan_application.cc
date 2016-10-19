// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/vulkan/vulkan_application.h"
#include "flutter/vulkan/vulkan_device.h"
#include "flutter/vulkan/vulkan_proc_table.h"

namespace vulkan {

VulkanApplication::VulkanApplication(
    const VulkanProcTable& p_vk,
    const std::string& application_name,
    const std::vector<std::string>& enabled_extensions,
    uint32_t application_version,
    uint32_t api_version)
    : vk(p_vk), api_version_(api_version), valid_(false) {
  const VkApplicationInfo info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = application_name.c_str(),
      .applicationVersion = application_version,
      .pEngineName = "FlutterEngine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = api_version_,
  };

  const char* extensions[enabled_extensions.size()];

  for (size_t i = 0; i < enabled_extensions.size(); i++) {
    extensions[i] = enabled_extensions[i].c_str();
  }

  const VkInstanceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &info,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size()),
      .ppEnabledExtensionNames = extensions,
  };

  VkInstance instance = VK_NULL_HANDLE;
  if (vk.createInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
    return;
  }

  instance_ = {instance,
               [this](VkInstance i) { vk.destroyInstance(i, nullptr); }};

  valid_ = true;
}

VulkanApplication::~VulkanApplication() = default;

bool VulkanApplication::IsValid() const {
  return valid_;
}

uint32_t VulkanApplication::APIVersion() const {
  return api_version_;
}

const VulkanHandle<VkInstance>& VulkanApplication::Instance() {
  return instance_;
}

std::vector<VkPhysicalDevice> VulkanApplication::GetPhysicalDevices() const {
  if (!IsValid()) {
    return {};
  }

  uint32_t device_count = 0;
  if (vk.enumeratePhysicalDevices(instance_, &device_count, nullptr) !=
      VK_SUCCESS) {
    return {};
  }

  if (device_count == 0) {
    // No available devices.
    return {};
  }

  std::vector<VkPhysicalDevice> physical_devices;

  physical_devices.resize(device_count);

  if (vk.enumeratePhysicalDevices(instance_, &device_count,
                                  physical_devices.data()) != VK_SUCCESS) {
    return {};
  }

  return physical_devices;
}

std::unique_ptr<VulkanDevice>
VulkanApplication::AcquireCompatibleLogicalDevice() const {
  for (auto device_handle : GetPhysicalDevices()) {
    auto logical_device = std::make_unique<VulkanDevice>(vk, device_handle);
    if (logical_device->IsValid()) {
      return logical_device;
    }
  }
  return nullptr;
}

}  // namespace vulkan
