#include "flutter/flutter_vma/flutter_vma.h"

#include "include/gpu/vk/GrVkBackendContext.h"

namespace flutter {

class FlutterSkiaVulkanMemoryAllocator : public skgpu::VulkanMemoryAllocator {
 public:
  static sk_sp<VulkanMemoryAllocator> Make(
      uint32_t vulkan_api_version,
      VkInstance instance,
      VkPhysicalDevice physicalDevice,
      VkDevice device,
      PFN_vkGetInstanceProcAddr get_instance_proc_address,
      PFN_vkGetDeviceProcAddr get_device_proc_address,
      bool mustUseCoherentHostVisibleMemory);

  ~FlutterSkiaVulkanMemoryAllocator() override;

  VkResult allocateImageMemory(VkImage image,
                               uint32_t allocationPropertyFlags,
                               skgpu::VulkanBackendMemory*) override;

  VkResult allocateBufferMemory(VkBuffer buffer,
                                BufferUsage usage,
                                uint32_t allocationPropertyFlags,
                                skgpu::VulkanBackendMemory*) override;

  void freeMemory(const skgpu::VulkanBackendMemory&) override;

  void getAllocInfo(const skgpu::VulkanBackendMemory&,
                    skgpu::VulkanAlloc*) const override;

  VkResult mapMemory(const skgpu::VulkanBackendMemory&, void** data) override;
  void unmapMemory(const skgpu::VulkanBackendMemory&) override;

  VkResult flushMemory(const skgpu::VulkanBackendMemory&,
                       VkDeviceSize offset,
                       VkDeviceSize size) override;
  VkResult invalidateMemory(const skgpu::VulkanBackendMemory&,
                            VkDeviceSize offset,
                            VkDeviceSize size) override;

  uint64_t totalUsedMemory() const override;
  uint64_t totalAllocatedMemory() const override;

 private:
  FlutterSkiaVulkanMemoryAllocator(VmaAllocator allocator,
                                   bool mustUseCoherentHostVisibleMemory);

  VmaAllocator allocator_;

  // For host visible allocations do we require they are coherent or not. All
  // devices are required to support a host visible and coherent memory type.
  // This is used to work around bugs for devices that don't handle non coherent
  // memory correctly.
  bool must_use_coherent_host_visible_memory_;
};

}  // namespace flutter
