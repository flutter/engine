// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flutter_vma/flutter_skia_vma.h"

#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/vulkan/procs/vulkan_handle.h"
#include "flutter/vulkan/procs/vulkan_proc_table.h"

namespace flutter {

sk_sp<skgpu::VulkanMemoryAllocator> FlutterSkiaVulkanMemoryAllocator::Make(
    uint32_t vulkan_api_version,
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    fml::RefPtr<vulkan::VulkanProcTable> vk,
    bool mustUseCoherentHostVisibleMemory) {
  FML_LOG(ERROR) << "here!!!";

  if (vk->AllocateMemory == nullptr) {
    FML_LOG(ERROR) << "Could not acquire vkAllocateMemory.";
    return nullptr;
  }

  // clang-format off
  VmaVulkanFunctions proc_table = {};
  proc_table.vkGetInstanceProcAddr = vk->NativeGetInstanceProcAddr();
  proc_table.vkGetDeviceProcAddr = vk->GetDeviceProcAddr;
  proc_table.vkGetPhysicalDeviceProperties = vk->GetPhysicalDeviceProperties;
  proc_table.vkGetPhysicalDeviceMemoryProperties = vk->GetPhysicalDeviceMemoryProperties;
  proc_table.vkAllocateMemory = vk->AllocateMemory;
  proc_table.vkFreeMemory = vk->FreeMemory;
  proc_table.vkMapMemory = vk->MapMemory;
  proc_table.vkUnmapMemory = vk->UnmapMemory;
  proc_table.vkFlushMappedMemoryRanges = vk->FlushMappedMemoryRanges;
  proc_table.vkInvalidateMappedMemoryRanges = vk->InvalidateMappedMemoryRanges;
  proc_table.vkBindBufferMemory = vk->BindBufferMemory;
  proc_table.vkBindImageMemory = vk->BindImageMemory;
  proc_table.vkGetBufferMemoryRequirements = vk->GetBufferMemoryRequirements;
  proc_table.vkGetImageMemoryRequirements = vk->GetImageMemoryRequirements;
  proc_table.vkCreateBuffer = vk->CreateBuffer;
  proc_table.vkDestroyBuffer = vk->DestroyBuffer;
  proc_table.vkCreateImage = vk->CreateImage;
  proc_table.vkDestroyImage = vk->DestroyImage;
  proc_table.vkCmdCopyBuffer = vk->CmdCopyBuffer;
  // clang-format on

  VmaAllocatorCreateInfo allocator_info = {};
  allocator_info.vulkanApiVersion = vulkan_api_version;
  allocator_info.physicalDevice = physicalDevice;
  allocator_info.device = device;
  allocator_info.instance = instance;
  allocator_info.pVulkanFunctions = &proc_table;

  VmaAllocator allocator;
  vmaCreateAllocator(&allocator_info, &allocator);

  return sk_sp<FlutterSkiaVulkanMemoryAllocator>(
      new FlutterSkiaVulkanMemoryAllocator(vk, allocator,
                                           mustUseCoherentHostVisibleMemory));
}

FlutterSkiaVulkanMemoryAllocator::FlutterSkiaVulkanMemoryAllocator(
    fml::RefPtr<vulkan::VulkanProcTable> vk_proc_table,
    VmaAllocator allocator,
    bool mustUseCoherentHostVisibleMemory)
    : vk_proc_table_(vk_proc_table),
      allocator_(allocator),
      must_use_coherent_host_visible_memory_(mustUseCoherentHostVisibleMemory) {
}

FlutterSkiaVulkanMemoryAllocator::~FlutterSkiaVulkanMemoryAllocator() {
  vmaDestroyAllocator(allocator_);
  allocator_ = VK_NULL_HANDLE;
}

VkResult FlutterSkiaVulkanMemoryAllocator::allocateImageMemory(
    VkImage image,
    uint32_t allocationPropertyFlags,
    skgpu::VulkanBackendMemory* backendMemory) {
  VmaAllocationCreateInfo info;
  info.flags = 0;
  info.usage = VMA_MEMORY_USAGE_UNKNOWN;
  info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  info.preferredFlags = 0;
  info.memoryTypeBits = 0;
  info.pool = VK_NULL_HANDLE;
  info.pUserData = nullptr;

  if (kDedicatedAllocation_AllocationPropertyFlag & allocationPropertyFlags) {
    info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  }
  if (kLazyAllocation_AllocationPropertyFlag & allocationPropertyFlags) {
    info.requiredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
  }
  if (kProtected_AllocationPropertyFlag & allocationPropertyFlags) {
    info.requiredFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
  }

  VmaAllocation allocation;
  VkResult result =
      vmaAllocateMemoryForImage(allocator_, image, &info, &allocation, nullptr);
  if (VK_SUCCESS == result) {
    *backendMemory = reinterpret_cast<skgpu::VulkanBackendMemory>(allocation);
  }
  return result;
}

VkResult FlutterSkiaVulkanMemoryAllocator::allocateBufferMemory(
    VkBuffer buffer,
    BufferUsage usage,
    uint32_t allocationPropertyFlags,
    skgpu::VulkanBackendMemory* backendMemory) {
  VmaAllocationCreateInfo info;
  info.flags = 0;
  info.usage = VMA_MEMORY_USAGE_UNKNOWN;
  info.memoryTypeBits = 0;
  info.pool = VK_NULL_HANDLE;
  info.pUserData = nullptr;

  switch (usage) {
    case BufferUsage::kGpuOnly:
      info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      info.preferredFlags = 0;
      break;
    case BufferUsage::kCpuWritesGpuReads:
      // When doing cpu writes and gpu reads the general rule of thumb is to use
      // coherent memory. Though this depends on the fact that we are not doing
      // any cpu reads and the cpu writes are sequential. For sparse writes we'd
      // want cpu cached memory, however we don't do these types of writes in
      // Skia.
      //
      // TODO (kaushikiska): In the future there may be times where specific
      // types of memory could benefit from a coherent and cached memory.
      // Typically these allow for the gpu to read cpu writes from the cache
      // without needing to flush the writes throughout the cache. The reverse
      // is not true and GPU writes tend to invalidate the cache regardless.
      // Also these gpu cache read access are typically lower bandwidth than
      // non-cached memory. For now Skia doesn't really have a need or want of
      // this type of memory. But if we ever do we could pass in an
      // AllocationPropertyFlag that requests the cached property.
      info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      break;
    case BufferUsage::kTransfersFromCpuToGpu:
      info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      info.preferredFlags = 0;
      break;
    case BufferUsage::kTransfersFromGpuToCpu:
      info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      info.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
      break;
  }

  if (must_use_coherent_host_visible_memory_ &&
      (info.requiredFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
    info.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }
  if (kDedicatedAllocation_AllocationPropertyFlag & allocationPropertyFlags) {
    info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  }
  if ((kLazyAllocation_AllocationPropertyFlag & allocationPropertyFlags) &&
      BufferUsage::kGpuOnly == usage) {
    info.preferredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
  }

  if (kPersistentlyMapped_AllocationPropertyFlag & allocationPropertyFlags) {
    SkASSERT(BufferUsage::kGpuOnly != usage);
    info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
  }

  VmaAllocation allocation;
  VkResult result = vmaAllocateMemoryForBuffer(allocator_, buffer, &info,
                                               &allocation, nullptr);
  if (VK_SUCCESS == result) {
    *backendMemory = reinterpret_cast<skgpu::VulkanBackendMemory>(allocation);
  }

  return result;
}

void FlutterSkiaVulkanMemoryAllocator::freeMemory(
    const skgpu::VulkanBackendMemory& memoryHandle) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  vmaFreeMemory(allocator_, allocation);
}

void FlutterSkiaVulkanMemoryAllocator::getAllocInfo(
    const skgpu::VulkanBackendMemory& memoryHandle,
    skgpu::VulkanAlloc* alloc) const {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  VmaAllocationInfo vmaInfo;
  vmaGetAllocationInfo(allocator_, allocation, &vmaInfo);

  VkMemoryPropertyFlags memFlags;
  vmaGetMemoryTypeProperties(allocator_, vmaInfo.memoryType, &memFlags);

  uint32_t flags = 0;
  if (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & memFlags) {
    flags |= skgpu::VulkanAlloc::kMappable_Flag;
  }
  if (!SkToBool(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & memFlags)) {
    flags |= skgpu::VulkanAlloc::kNoncoherent_Flag;
  }
  if (VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT & memFlags) {
    flags |= skgpu::VulkanAlloc::kLazilyAllocated_Flag;
  }

  alloc->fMemory = vmaInfo.deviceMemory;
  alloc->fOffset = vmaInfo.offset;
  alloc->fSize = vmaInfo.size;
  alloc->fFlags = flags;
  alloc->fBackendMemory = memoryHandle;
}

VkResult FlutterSkiaVulkanMemoryAllocator::mapMemory(
    const skgpu::VulkanBackendMemory& memoryHandle,
    void** data) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  return vmaMapMemory(allocator_, allocation, data);
}

void FlutterSkiaVulkanMemoryAllocator::unmapMemory(
    const skgpu::VulkanBackendMemory& memoryHandle) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  vmaUnmapMemory(allocator_, allocation);
}

VkResult FlutterSkiaVulkanMemoryAllocator::flushMemory(
    const skgpu::VulkanBackendMemory& memoryHandle,
    VkDeviceSize offset,
    VkDeviceSize size) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  return vmaFlushAllocation(allocator_, allocation, offset, size);
}

VkResult FlutterSkiaVulkanMemoryAllocator::invalidateMemory(
    const skgpu::VulkanBackendMemory& memoryHandle,
    VkDeviceSize offset,
    VkDeviceSize size) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  return vmaInvalidateAllocation(allocator_, allocation, offset, size);
}

uint64_t FlutterSkiaVulkanMemoryAllocator::totalUsedMemory() const {
  VmaTotalStatistics stats;
  vmaCalculateStatistics(allocator_, &stats);
  return stats.total.statistics.allocationBytes;
}

uint64_t FlutterSkiaVulkanMemoryAllocator::totalAllocatedMemory() const {
  VmaTotalStatistics stats;
  vmaCalculateStatistics(allocator_, &stats);
  return stats.total.statistics.blockBytes;
}

}  // namespace flutter
