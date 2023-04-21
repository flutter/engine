// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/renderer/backend/vulkan/blit_command_vk.h"
#include "impeller/renderer/backend/vulkan/command_encoder_vk.h"

namespace impeller {
namespace testing {

namespace {
void noop() {}

VkResult vkEnumerateInstanceExtensionProperties(
    const char* pLayerName,
    uint32_t* pPropertyCount,
    VkExtensionProperties* pProperties) {
  if (!pProperties) {
    *pPropertyCount = 2;

  } else {
    strcpy(pProperties[0].extensionName, "VK_KHR_surface");
    pProperties[0].specVersion = 0;
    strcpy(pProperties[1].extensionName, "VK_MVK_macos_surface");
    pProperties[1].specVersion = 0;
  }
  return VK_SUCCESS;
}

VkResult vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount,
                                            VkLayerProperties* pProperties) {
  *pPropertyCount = 0;
  return VK_SUCCESS;
}

VkResult vkEnumeratePhysicalDevices(VkInstance instance,
                                    uint32_t* pPhysicalDeviceCount,
                                    VkPhysicalDevice* pPhysicalDevices) {
  if (!pPhysicalDevices) {
    *pPhysicalDeviceCount = 1;
  } else {
    pPhysicalDevices[0] = reinterpret_cast<VkPhysicalDevice>(0xfeedface);
  }
  return VK_SUCCESS;
}

void vkGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkFormatProperties* pFormatProperties) {
  if (format == VK_FORMAT_B8G8R8A8_UNORM) {
    pFormatProperties->optimalTilingFeatures =
        static_cast<VkFormatFeatureFlags>(
            vk::FormatFeatureFlagBits::eColorAttachment);
  } else if (format == VK_FORMAT_S8_UINT) {
    pFormatProperties->optimalTilingFeatures =
        static_cast<VkFormatFeatureFlags>(
            vk::FormatFeatureFlagBits::eDepthStencilAttachment);
  }
}

void vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                   VkPhysicalDeviceProperties* pProperties) {
  pProperties->limits.framebufferColorSampleCounts =
      static_cast<VkSampleCountFlags>(VK_SAMPLE_COUNT_1_BIT |
                                      VK_SAMPLE_COUNT_4_BIT);
}

void vkGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice physicalDevice,
    uint32_t* pQueueFamilyPropertyCount,
    VkQueueFamilyProperties* pQueueFamilyProperties) {
  if (!pQueueFamilyProperties) {
    *pQueueFamilyPropertyCount = 1;
  } else {
    pQueueFamilyProperties[0].queueCount = 3;
    pQueueFamilyProperties[0].queueFlags = static_cast<VkQueueFlags>(
        VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT);
  }
}

VkResult vkEnumerateDeviceExtensionProperties(
    VkPhysicalDevice physicalDevice,
    const char* pLayerName,
    uint32_t* pPropertyCount,
    VkExtensionProperties* pProperties) {
  if (!pProperties) {
    *pPropertyCount = 1;
  } else {
    strcpy(pProperties[0].extensionName, "VK_KHR_swapchain");
    pProperties[0].specVersion = 0;
  }
  return VK_SUCCESS;
}

VkResult vkCreateDevice(VkPhysicalDevice physicalDevice,
                        const VkDeviceCreateInfo* pCreateInfo,
                        const VkAllocationCallbacks* pAllocator,
                        VkDevice* pDevice) {
  *pDevice = reinterpret_cast<VkDevice>(0xcafebabe);
  return VK_SUCCESS;
}

VkResult vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                          const VkAllocationCallbacks* pAllocator,
                          VkInstance* pInstance) {
  *pInstance = reinterpret_cast<VkInstance>(0xbaadf00d);
  return VK_SUCCESS;
}

void vkGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
  pMemoryProperties->memoryTypeCount = 1;
  pMemoryProperties->memoryTypes[0].heapIndex = 0;
  pMemoryProperties->memoryTypes[0].propertyFlags = 0;
  pMemoryProperties->memoryHeapCount = 1;
  pMemoryProperties->memoryHeaps[0].size = 1024 * 1024 * 1024;
  pMemoryProperties->memoryHeaps[0].flags = 0;
}

VkResult vkCreatePipelineCache(VkDevice device,
                               const VkPipelineCacheCreateInfo* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator,
                               VkPipelineCache* pPipelineCache) {
  *pPipelineCache = reinterpret_cast<VkPipelineCache>(0xb000dead);
  return VK_SUCCESS;
}

}  // namespace

TEST(BlitCommandVkTest, BlitCopyTextureToTextureCommandVK) {
  ContextVK::Settings settings;
  auto message_loop = fml::ConcurrentMessageLoop::Create();
  settings.worker_task_runner =
      std::make_shared<fml::ConcurrentTaskRunner>(message_loop);
  settings.proc_address_callback = [](VkInstance instance,
                                      const char* pName) -> PFN_vkVoidFunction {
    if (strcmp("vkEnumerateInstanceExtensionProperties", pName) == 0) {
      return (PFN_vkVoidFunction)vkEnumerateInstanceExtensionProperties;
    } else if (strcmp("vkEnumerateInstanceLayerProperties", pName) == 0) {
      return (PFN_vkVoidFunction)vkEnumerateInstanceLayerProperties;
    } else if (strcmp("vkEnumeratePhysicalDevices", pName) == 0) {
      return (PFN_vkVoidFunction)vkEnumeratePhysicalDevices;
    } else if (strcmp("vkGetPhysicalDeviceFormatProperties", pName) == 0) {
      return (PFN_vkVoidFunction)vkGetPhysicalDeviceFormatProperties;
    } else if (strcmp("vkGetPhysicalDeviceProperties", pName) == 0) {
      return (PFN_vkVoidFunction)vkGetPhysicalDeviceProperties;
    } else if (strcmp("vkGetPhysicalDeviceQueueFamilyProperties", pName) == 0) {
      return (PFN_vkVoidFunction)vkGetPhysicalDeviceQueueFamilyProperties;
    } else if (strcmp("vkEnumerateDeviceExtensionProperties", pName) == 0) {
      return (PFN_vkVoidFunction)vkEnumerateDeviceExtensionProperties;
    } else if (strcmp("vkCreateDevice", pName) == 0) {
      return (PFN_vkVoidFunction)vkCreateDevice;
    } else if (strcmp("vkCreateInstance", pName) == 0) {
      return (PFN_vkVoidFunction)vkCreateInstance;
    } else if (strcmp("vkGetPhysicalDeviceMemoryProperties", pName) == 0) {
      return (PFN_vkVoidFunction)vkGetPhysicalDeviceMemoryProperties;
    } else if (strcmp("vkCreatePipelineCache", pName) == 0) {
      return (PFN_vkVoidFunction)vkCreatePipelineCache;
    }
    return noop;
  };
  auto context = ContextVK::Create(std::move(settings));
  CommandEncoderVK encoder(context->GetDevice(), {}, {},
                           context->GetFenceWaiter());
  BlitCopyTextureToTextureCommandVK cmd;
  bool result = cmd.Encode(encoder);
  ASSERT_TRUE(result);
}

}  // namespace testing
}  // namespace impeller
