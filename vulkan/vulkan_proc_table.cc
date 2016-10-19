// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "lib/ftl/logging.h"
#include "vulkan_proc_table.h"

#include <dlfcn.h>

namespace vulkan {

VulkanProcTable::VulkanProcTable() : valid_(false), handle_(nullptr) {
  if (!OpenLibraryHandle()) {
    return;
  }

  if (!AcquireProcs()) {
    return;
  }

  valid_ = true;
}

VulkanProcTable::~VulkanProcTable() {
  CloseLibraryHandle();
}

bool VulkanProcTable::IsValid() const {
  return valid_;
}

bool VulkanProcTable::OpenLibraryHandle() {
  dlerror();  // clear existing errors on thread.
  handle_ = dlopen("libvulkan.so", RTLD_NOW);
  if (handle_ == nullptr) {
    FTL_DLOG(WARNING) << "Could not open the vulkan library: " << dlerror();
    return false;
  }
  return true;
}

bool VulkanProcTable::CloseLibraryHandle() {
  if (handle_ != nullptr) {
    dlerror();  // clear existing errors on thread.
    if (dlclose(handle_) != 0) {
      FTL_DLOG(ERROR) << "Could not close the vulkan library handle. This "
                         "indicates a leak.";
      FTL_DLOG(ERROR) << dlerror();
    }
    handle_ = nullptr;
  }

  return handle_ == nullptr;
}

bool VulkanProcTable::AcquireProcs() {
  if (handle_ == nullptr) {
    return false;
  }

#define ACQUIRE_PROC(symbol, name)                                       \
  if (!(symbol = reinterpret_cast<decltype(symbol)::Proto>(              \
            dlsym(handle_, name)))) {                                    \
    FTL_DLOG(WARNING) << "Could not acquire proc for function " << name; \
    return false;                                                        \
  }

  ACQUIRE_PROC(createInstance, "vkCreateInstance");
  ACQUIRE_PROC(destroyInstance, "vkDestroyInstance");
  ACQUIRE_PROC(enumeratePhysicalDevices, "vkEnumeratePhysicalDevices");
  ACQUIRE_PROC(createDevice, "vkCreateDevice");
  ACQUIRE_PROC(destroyDevice, "vkDestroyDevice");
  ACQUIRE_PROC(getDeviceQueue, "vkGetDeviceQueue");
  ACQUIRE_PROC(getPhysicalDeviceSurfaceCapabilitiesKHR,
               "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
  ACQUIRE_PROC(getPhysicalDeviceSurfaceFormatsKHR,
               "vkGetPhysicalDeviceSurfaceFormatsKHR");
  ACQUIRE_PROC(createSwapchainKHR, "vkCreateSwapchainKHR");
  ACQUIRE_PROC(getSwapchainImagesKHR, "vkGetSwapchainImagesKHR");
  ACQUIRE_PROC(getPhysicalDeviceSurfacePresentModesKHR,
               "vkGetPhysicalDeviceSurfacePresentModesKHR");
  ACQUIRE_PROC(destroySurfaceKHR, "vkDestroySurfaceKHR");
  ACQUIRE_PROC(createCommandPool, "vkCreateCommandPool");
  ACQUIRE_PROC(destroyCommandPool, "vkDestroyCommandPool");
  ACQUIRE_PROC(createSemaphore, "vkCreateSemaphore");
  ACQUIRE_PROC(destroySemaphore, "vkDestroySemaphore");
  ACQUIRE_PROC(allocateCommandBuffers, "vkAllocateCommandBuffers");
  ACQUIRE_PROC(freeCommandBuffers, "vkFreeCommandBuffers");
  ACQUIRE_PROC(createFence, "vkCreateFence");
  ACQUIRE_PROC(destroyFence, "vkDestroyFence");
  ACQUIRE_PROC(waitForFences, "vkWaitForFences");
  ACQUIRE_PROC(resetFences, "vkResetFences");
  ACQUIRE_PROC(acquireNextImageKHR, "vkAcquireNextImageKHR");
  ACQUIRE_PROC(resetCommandBuffer, "vkResetCommandBuffer");
  ACQUIRE_PROC(beginCommandBuffer, "vkBeginCommandBuffer");
  ACQUIRE_PROC(cmdPipelineBarrier, "vkCmdPipelineBarrier");
  ACQUIRE_PROC(endCommandBuffer, "vkEndCommandBuffer");
  ACQUIRE_PROC(queueSubmit, "vkQueueSubmit");
  ACQUIRE_PROC(getPhysicalDeviceQueueFamilyProperties,
               "vkGetPhysicalDeviceQueueFamilyProperties");
  ACQUIRE_PROC(deviceWaitIdle, "vkDeviceWaitIdle");
  ACQUIRE_PROC(getPhysicalDeviceFeatures, "vkGetPhysicalDeviceFeatures");
  ACQUIRE_PROC(queuePresentKHR, "vkQueuePresentKHR");
  ACQUIRE_PROC(destroySwapchainKHR, "vkDestroySwapchainKHR");

#if OS_ANDROID
  ACQUIRE_PROC(createAndroidSurfaceKHR, "vkCreateAndroidSurfaceKHR");
#endif  // OS_ANDROID

#undef ACQUIRE_PROC

  return true;
}

void* VulkanProcTable::AcquireProcByName(const char* proc_name) const {
  return proc_name == nullptr ? nullptr : dlsym(handle_, proc_name);
}

SkAutoTUnref<GrVkInterface> VulkanProcTable::SkiaInterface() const {
  auto interface = SkAutoTUnref<GrVkInterface>(new GrVkInterface());

#define ACQUIRE_SK(member, name) \
  interface->fFunctions.member = \
      reinterpret_cast<PFN_##name>(AcquireProcByName(#name))

  ACQUIRE_SK(fCreateInstance, vkCreateInstance);
  ACQUIRE_SK(fDestroyInstance, vkDestroyInstance);
  ACQUIRE_SK(fEnumeratePhysicalDevices, vkEnumeratePhysicalDevices);
  ACQUIRE_SK(fGetPhysicalDeviceFeatures, vkGetPhysicalDeviceFeatures);
  ACQUIRE_SK(fGetPhysicalDeviceFormatProperties,
             vkGetPhysicalDeviceFormatProperties);
  ACQUIRE_SK(fGetPhysicalDeviceImageFormatProperties,
             vkGetPhysicalDeviceImageFormatProperties);
  ACQUIRE_SK(fGetPhysicalDeviceProperties, vkGetPhysicalDeviceProperties);
  ACQUIRE_SK(fGetPhysicalDeviceQueueFamilyProperties,
             vkGetPhysicalDeviceQueueFamilyProperties);
  ACQUIRE_SK(fGetPhysicalDeviceMemoryProperties,
             vkGetPhysicalDeviceMemoryProperties);
  ACQUIRE_SK(fCreateDevice, vkCreateDevice);
  ACQUIRE_SK(fDestroyDevice, vkDestroyDevice);
  ACQUIRE_SK(fEnumerateInstanceExtensionProperties,
             vkEnumerateInstanceExtensionProperties);
  ACQUIRE_SK(fEnumerateDeviceExtensionProperties,
             vkEnumerateDeviceExtensionProperties);
  ACQUIRE_SK(fEnumerateInstanceLayerProperties,
             vkEnumerateInstanceLayerProperties);
  ACQUIRE_SK(fEnumerateDeviceLayerProperties, vkEnumerateDeviceLayerProperties);
  ACQUIRE_SK(fGetDeviceQueue, vkGetDeviceQueue);
  ACQUIRE_SK(fQueueSubmit, vkQueueSubmit);
  ACQUIRE_SK(fQueueWaitIdle, vkQueueWaitIdle);
  ACQUIRE_SK(fDeviceWaitIdle, vkDeviceWaitIdle);
  ACQUIRE_SK(fAllocateMemory, vkAllocateMemory);
  ACQUIRE_SK(fFreeMemory, vkFreeMemory);
  ACQUIRE_SK(fMapMemory, vkMapMemory);
  ACQUIRE_SK(fUnmapMemory, vkUnmapMemory);
  ACQUIRE_SK(fFlushMappedMemoryRanges, vkFlushMappedMemoryRanges);
  ACQUIRE_SK(fInvalidateMappedMemoryRanges, vkInvalidateMappedMemoryRanges);
  ACQUIRE_SK(fGetDeviceMemoryCommitment, vkGetDeviceMemoryCommitment);
  ACQUIRE_SK(fBindBufferMemory, vkBindBufferMemory);
  ACQUIRE_SK(fBindImageMemory, vkBindImageMemory);
  ACQUIRE_SK(fGetBufferMemoryRequirements, vkGetBufferMemoryRequirements);
  ACQUIRE_SK(fGetImageMemoryRequirements, vkGetImageMemoryRequirements);
  ACQUIRE_SK(fGetImageSparseMemoryRequirements,
             vkGetImageSparseMemoryRequirements);
  ACQUIRE_SK(fGetPhysicalDeviceSparseImageFormatProperties,
             vkGetPhysicalDeviceSparseImageFormatProperties);
  ACQUIRE_SK(fQueueBindSparse, vkQueueBindSparse);
  ACQUIRE_SK(fCreateFence, vkCreateFence);
  ACQUIRE_SK(fDestroyFence, vkDestroyFence);
  ACQUIRE_SK(fResetFences, vkResetFences);
  ACQUIRE_SK(fGetFenceStatus, vkGetFenceStatus);
  ACQUIRE_SK(fWaitForFences, vkWaitForFences);
  ACQUIRE_SK(fCreateSemaphore, vkCreateSemaphore);
  ACQUIRE_SK(fDestroySemaphore, vkDestroySemaphore);
  ACQUIRE_SK(fCreateEvent, vkCreateEvent);
  ACQUIRE_SK(fDestroyEvent, vkDestroyEvent);
  ACQUIRE_SK(fGetEventStatus, vkGetEventStatus);
  ACQUIRE_SK(fSetEvent, vkSetEvent);
  ACQUIRE_SK(fResetEvent, vkResetEvent);
  ACQUIRE_SK(fCreateQueryPool, vkCreateQueryPool);
  ACQUIRE_SK(fDestroyQueryPool, vkDestroyQueryPool);
  ACQUIRE_SK(fGetQueryPoolResults, vkGetQueryPoolResults);
  ACQUIRE_SK(fCreateBuffer, vkCreateBuffer);
  ACQUIRE_SK(fDestroyBuffer, vkDestroyBuffer);
  ACQUIRE_SK(fCreateBufferView, vkCreateBufferView);
  ACQUIRE_SK(fDestroyBufferView, vkDestroyBufferView);
  ACQUIRE_SK(fCreateImage, vkCreateImage);
  ACQUIRE_SK(fDestroyImage, vkDestroyImage);
  ACQUIRE_SK(fGetImageSubresourceLayout, vkGetImageSubresourceLayout);
  ACQUIRE_SK(fCreateImageView, vkCreateImageView);
  ACQUIRE_SK(fDestroyImageView, vkDestroyImageView);
  ACQUIRE_SK(fCreateShaderModule, vkCreateShaderModule);
  ACQUIRE_SK(fDestroyShaderModule, vkDestroyShaderModule);
  ACQUIRE_SK(fCreatePipelineCache, vkCreatePipelineCache);
  ACQUIRE_SK(fDestroyPipelineCache, vkDestroyPipelineCache);
  ACQUIRE_SK(fGetPipelineCacheData, vkGetPipelineCacheData);
  ACQUIRE_SK(fMergePipelineCaches, vkMergePipelineCaches);
  ACQUIRE_SK(fCreateGraphicsPipelines, vkCreateGraphicsPipelines);
  ACQUIRE_SK(fCreateComputePipelines, vkCreateComputePipelines);
  ACQUIRE_SK(fDestroyPipeline, vkDestroyPipeline);
  ACQUIRE_SK(fCreatePipelineLayout, vkCreatePipelineLayout);
  ACQUIRE_SK(fDestroyPipelineLayout, vkDestroyPipelineLayout);
  ACQUIRE_SK(fCreateSampler, vkCreateSampler);
  ACQUIRE_SK(fDestroySampler, vkDestroySampler);
  ACQUIRE_SK(fCreateDescriptorSetLayout, vkCreateDescriptorSetLayout);
  ACQUIRE_SK(fDestroyDescriptorSetLayout, vkDestroyDescriptorSetLayout);
  ACQUIRE_SK(fCreateDescriptorPool, vkCreateDescriptorPool);
  ACQUIRE_SK(fDestroyDescriptorPool, vkDestroyDescriptorPool);
  ACQUIRE_SK(fResetDescriptorPool, vkResetDescriptorPool);
  ACQUIRE_SK(fAllocateDescriptorSets, vkAllocateDescriptorSets);
  ACQUIRE_SK(fFreeDescriptorSets, vkFreeDescriptorSets);
  ACQUIRE_SK(fUpdateDescriptorSets, vkUpdateDescriptorSets);
  ACQUIRE_SK(fCreateFramebuffer, vkCreateFramebuffer);
  ACQUIRE_SK(fDestroyFramebuffer, vkDestroyFramebuffer);
  ACQUIRE_SK(fCreateRenderPass, vkCreateRenderPass);
  ACQUIRE_SK(fDestroyRenderPass, vkDestroyRenderPass);
  ACQUIRE_SK(fGetRenderAreaGranularity, vkGetRenderAreaGranularity);
  ACQUIRE_SK(fCreateCommandPool, vkCreateCommandPool);
  ACQUIRE_SK(fDestroyCommandPool, vkDestroyCommandPool);
  ACQUIRE_SK(fResetCommandPool, vkResetCommandPool);
  ACQUIRE_SK(fAllocateCommandBuffers, vkAllocateCommandBuffers);
  ACQUIRE_SK(fFreeCommandBuffers, vkFreeCommandBuffers);
  ACQUIRE_SK(fBeginCommandBuffer, vkBeginCommandBuffer);
  ACQUIRE_SK(fEndCommandBuffer, vkEndCommandBuffer);
  ACQUIRE_SK(fResetCommandBuffer, vkResetCommandBuffer);
  ACQUIRE_SK(fCmdBindPipeline, vkCmdBindPipeline);
  ACQUIRE_SK(fCmdSetViewport, vkCmdSetViewport);
  ACQUIRE_SK(fCmdSetScissor, vkCmdSetScissor);
  ACQUIRE_SK(fCmdSetLineWidth, vkCmdSetLineWidth);
  ACQUIRE_SK(fCmdSetDepthBias, vkCmdSetDepthBias);
  ACQUIRE_SK(fCmdSetBlendConstants, vkCmdSetBlendConstants);
  ACQUIRE_SK(fCmdSetDepthBounds, vkCmdSetDepthBounds);
  ACQUIRE_SK(fCmdSetStencilCompareMask, vkCmdSetStencilCompareMask);
  ACQUIRE_SK(fCmdSetStencilWriteMask, vkCmdSetStencilWriteMask);
  ACQUIRE_SK(fCmdSetStencilReference, vkCmdSetStencilReference);
  ACQUIRE_SK(fCmdBindDescriptorSets, vkCmdBindDescriptorSets);
  ACQUIRE_SK(fCmdBindIndexBuffer, vkCmdBindIndexBuffer);
  ACQUIRE_SK(fCmdBindVertexBuffers, vkCmdBindVertexBuffers);
  ACQUIRE_SK(fCmdDraw, vkCmdDraw);
  ACQUIRE_SK(fCmdDrawIndexed, vkCmdDrawIndexed);
  ACQUIRE_SK(fCmdDrawIndirect, vkCmdDrawIndirect);
  ACQUIRE_SK(fCmdDrawIndexedIndirect, vkCmdDrawIndexedIndirect);
  ACQUIRE_SK(fCmdDispatch, vkCmdDispatch);
  ACQUIRE_SK(fCmdDispatchIndirect, vkCmdDispatchIndirect);
  ACQUIRE_SK(fCmdCopyBuffer, vkCmdCopyBuffer);
  ACQUIRE_SK(fCmdCopyImage, vkCmdCopyImage);
  ACQUIRE_SK(fCmdBlitImage, vkCmdBlitImage);
  ACQUIRE_SK(fCmdCopyBufferToImage, vkCmdCopyBufferToImage);
  ACQUIRE_SK(fCmdCopyImageToBuffer, vkCmdCopyImageToBuffer);
  ACQUIRE_SK(fCmdUpdateBuffer, vkCmdUpdateBuffer);
  ACQUIRE_SK(fCmdFillBuffer, vkCmdFillBuffer);
  ACQUIRE_SK(fCmdClearColorImage, vkCmdClearColorImage);
  ACQUIRE_SK(fCmdClearDepthStencilImage, vkCmdClearDepthStencilImage);
  ACQUIRE_SK(fCmdClearAttachments, vkCmdClearAttachments);
  ACQUIRE_SK(fCmdResolveImage, vkCmdResolveImage);
  ACQUIRE_SK(fCmdSetEvent, vkCmdSetEvent);
  ACQUIRE_SK(fCmdResetEvent, vkCmdResetEvent);
  ACQUIRE_SK(fCmdWaitEvents, vkCmdWaitEvents);
  ACQUIRE_SK(fCmdPipelineBarrier, vkCmdPipelineBarrier);
  ACQUIRE_SK(fCmdBeginQuery, vkCmdBeginQuery);
  ACQUIRE_SK(fCmdEndQuery, vkCmdEndQuery);
  ACQUIRE_SK(fCmdResetQueryPool, vkCmdResetQueryPool);
  ACQUIRE_SK(fCmdWriteTimestamp, vkCmdWriteTimestamp);
  ACQUIRE_SK(fCmdCopyQueryPoolResults, vkCmdCopyQueryPoolResults);
  ACQUIRE_SK(fCmdPushConstants, vkCmdPushConstants);
  ACQUIRE_SK(fCmdBeginRenderPass, vkCmdBeginRenderPass);
  ACQUIRE_SK(fCmdNextSubpass, vkCmdNextSubpass);
  ACQUIRE_SK(fCmdEndRenderPass, vkCmdEndRenderPass);
  ACQUIRE_SK(fCmdExecuteCommands, vkCmdExecuteCommands);
  ACQUIRE_SK(fCreateDebugReportCallbackEXT, vkCreateDebugReportCallbackEXT);
  ACQUIRE_SK(fDebugReportMessageEXT, vkDebugReportMessageEXT);
  ACQUIRE_SK(fDestroyDebugReportCallbackEXT, vkDestroyDebugReportCallbackEXT);

#undef ACQUIRE_SK

  return interface;
}

}  // namespace vulkan
