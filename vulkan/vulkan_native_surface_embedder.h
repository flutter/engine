// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_EMBEDDER_H_
#define FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_EMBEDDER_H_

#include "flutter/fml/macros.h"
#include "flutter/vulkan/vulkan_native_surface.h"

namespace vulkan {

class VulkanNativeSurfaceEmbedder : public VulkanNativeSurface {
 public:
  struct VulkanDispatchTable {
    std::function<const char*()> get_extension_name;    // required
    std::function<uint32_t()> get_skia_extension_name;  // required
    std::function<SkISize()> get_size;                  // required
    std::function<VkSurfaceKHR(VulkanProcTable&, const VkInstance&)>
        create_surface_handle;  // required
  };

  VulkanNativeSurfaceEmbedder(VulkanDispatchTable dispatch_table);

  ~VulkanNativeSurfaceEmbedder();

  // |VulkanNativeSurface|
  const char* GetExtensionName() const override;

  // |VulkanNativeSurface|
  uint32_t GetSkiaExtensionName() const override;

  // |VulkanNativeSurface|
  VkSurfaceKHR CreateSurfaceHandle(
      VulkanProcTable& vk,
      const VulkanHandle<VkInstance>& instance) const override;

  // |VulkanNativeSurface|
  bool IsValid() const override;

  // |VulkanNativeSurface|
  SkISize GetSize() const override;

 private:
  VulkanDispatchTable dispatch_table_;
  bool valid_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(VulkanNativeSurfaceEmbedder);
};

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_EMBEDDER_H_
