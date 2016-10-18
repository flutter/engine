// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_SWAPCHAIN_H_
#define FLUTTER_VULKAN_VULKAN_SWAPCHAIN_H_

#include <memory>
#include <vector>
#include "flutter/vulkan/vulkan_handle.h"
#include "lib/ftl/macros.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace vulkan {

class VulkanProcTable;
class VulkanDevice;
class VulkanSurface;
class VulkanBackbuffer;
class VulkanImage;

class VulkanSwapchain {
 public:
  VulkanSwapchain(const VulkanProcTable& vk,
                  const VulkanDevice& device,
                  const VulkanSurface& surface,
                  GrContext* skia_context,
                  std::unique_ptr<VulkanSwapchain> old_swapchain);

  ~VulkanSwapchain();

  bool IsValid() const;

  enum class AcquireType {
    Success,
    ErrorSurfaceLost,
    ErrorSurfaceOutOfDate,
  };
  using AcquireResult = std::pair<AcquireType, sk_sp<SkSurface>>;

  AcquireResult AcquireSurface();

  FTL_WARN_UNUSED_RESULT
  bool Submit();

  SkISize GetSize() const;

 private:
  const VulkanProcTable& vk;
  const VulkanDevice& device_;
  VkSurfaceCapabilitiesKHR capabilities_;
  VkSurfaceFormatKHR surface_format_;
  VkPresentModeKHR present_mode_;
  VulkanHandle<VkSwapchainKHR> swapchain_;
  std::vector<std::unique_ptr<VulkanBackbuffer>> backbuffers_;
  std::vector<std::unique_ptr<VulkanImage>> images_;
  std::vector<sk_sp<SkSurface>> surfaces_;
  VkPipelineStageFlagBits current_pipeline_stage_;
  size_t current_backbuffer_index_;
  size_t current_image_index_;
  bool valid_;

  std::vector<VkImage> GetImages() const;

  bool CreateSwapchainImages(GrContext* skia_context);

  sk_sp<SkSurface> CreateSkiaSurface(GrContext* skia_context,
                                     VkImage image,
                                     const SkISize& size) const;

  VulkanBackbuffer* NextBackbuffer();

  FTL_DISALLOW_COPY_AND_ASSIGN(VulkanSwapchain);
};

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_SWAPCHAIN_H_
