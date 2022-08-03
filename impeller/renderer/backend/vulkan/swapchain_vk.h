// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <vector>
#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/device_buffer.h"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {

class SwapchainDetailsVK {
 public:
  static std::shared_ptr<SwapchainDetailsVK> Create(
      vk::SurfaceKHR surface,
      vk::PhysicalDevice physical_device);

  SwapchainDetailsVK(vk::SurfaceCapabilitiesKHR capabilities,
                     std::vector<vk::SurfaceFormatKHR> surface_formats,
                     std::vector<vk::PresentModeKHR> surface_present_modes);

  ~SwapchainDetailsVK();

  vk::SurfaceCapabilitiesKHR surface_capabilities_;
  std::vector<vk::SurfaceFormatKHR> surface_formats_;
  std::vector<vk::PresentModeKHR> present_modes_;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(SwapchainDetailsVK);
};

class SwapchainImage {
 public:
  SwapchainImage(vk::Image image, vk::UniqueImageView view);

  ~SwapchainImage();

 private:
  vk::Image image_;
  vk::UniqueImageView image_view_;

  FML_DISALLOW_COPY_AND_ASSIGN(SwapchainImage);
};

class SwapchainVK {
 public:
  static std::shared_ptr<SwapchainVK> Create(vk::Device device,
                                             vk::SurfaceKHR surface,
                                             SwapchainDetailsVK& details);

  SwapchainVK(vk::UniqueSwapchainKHR swapchain,
              vk::Format image_format,
              vk::Extent2D extent);

  ~SwapchainVK();

  void InitializeSwapchainImages(vk::Device device);

 private:
  vk::UniqueSwapchainKHR swapchain_;
  vk::Format image_format_;
  vk::Extent2D extent_;
  std::vector<std::unique_ptr<SwapchainImage>> swapchain_images_;

  FML_DISALLOW_COPY_AND_ASSIGN(SwapchainVK);
};

}  // namespace impeller
