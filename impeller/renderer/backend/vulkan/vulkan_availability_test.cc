#include <iostream>
#include "flutter/impeller/renderer/backend/vulkan/vk.h"
#include "flutter/testing/testing.h"
#include "gtest/gtest.h"

using namespace impeller;

TEST(VulkanAvailability, CreateInstance) {
  vk::DynamicLoader dl;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
      dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
  VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

  vk::ApplicationInfo appInfo("Hello Vulkan", VK_MAKE_VERSION(1, 0, 0),
                              "No Engine", VK_MAKE_VERSION(1, 0, 0),
                              VK_API_VERSION_1_0);
  vk::InstanceCreateInfo createInfo({}, &appInfo);
  createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

  auto instance_result = vk::createInstanceUnique(createInfo);
  ASSERT_EQ(instance_result.result, vk::Result::eSuccess);

  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_result.value.get());

  // Enumerate physical devices
  auto devices_result = instance_result.value->enumeratePhysicalDevices();
  ASSERT_EQ(devices_result.result, vk::Result::eSuccess);
  ASSERT_FALSE(devices_result.value.empty());

  std::cout << "Driver Info:" << std::endl;
  for (const auto& device : devices_result.value) {
    vk::PhysicalDeviceProperties deviceProperties = device.getProperties();

    std::cout << "Driver Name: " << deviceProperties.deviceName << std::endl;
    std::cout << "Driver Type: " << vk::to_string(deviceProperties.deviceType)
              << std::endl;
    std::cout << "API Version: "
              << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
              << VK_VERSION_MINOR(deviceProperties.apiVersion) << "."
              << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;
  }
}