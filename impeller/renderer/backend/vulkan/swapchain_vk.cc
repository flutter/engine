// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/swapchain_vk.h"
#include "fml/logging.h"
#include "impeller/base/validation.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {

std::shared_ptr<SwapchainDetailsVK> SwapchainDetailsVK::Create(
    vk::SurfaceKHR surface,
    vk::PhysicalDevice physical_device) {
  FML_DCHECK(surface) << "surface provided as nullptr";

  auto capabilities_res = physical_device.getSurfaceCapabilitiesKHR(surface);
  if (capabilities_res.result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to get surface capabilities: "
                   << vk::to_string(capabilities_res.result);
    return nullptr;
  }
  vk::SurfaceCapabilitiesKHR capabilities = capabilities_res.value;

  auto surface_formats_res = physical_device.getSurfaceFormatsKHR(surface);
  if (surface_formats_res.result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to get surface formats: "
                   << vk::to_string(surface_formats_res.result);
    return nullptr;
  }
  std::vector<vk::SurfaceFormatKHR> surface_formats = surface_formats_res.value;

  auto surface_present_modes_res =
      physical_device.getSurfacePresentModesKHR(surface);
  if (surface_present_modes_res.result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to get surface present modes: "
                   << vk::to_string(surface_present_modes_res.result);
    return nullptr;
  }
  std::vector<vk::PresentModeKHR> surface_present_modes =
      surface_present_modes_res.value;

  return std::make_shared<SwapchainDetailsVK>(capabilities, surface_formats,
                                              surface_present_modes);
}

SwapchainDetailsVK::SwapchainDetailsVK(
    vk::SurfaceCapabilitiesKHR capabilities,
    std::vector<vk::SurfaceFormatKHR> surface_formats,
    std::vector<vk::PresentModeKHR> surface_present_modes)
    : surface_capabilities_(capabilities),
      surface_formats_(surface_formats),
      present_modes_(surface_present_modes) {}

SwapchainDetailsVK::~SwapchainDetailsVK() = default;

static vk::SurfaceFormatKHR PickSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> formats) {
  for (const auto& format : formats) {
    if ((format.format == vk::Format::eR8G8B8A8Unorm ||
         format.format == vk::Format::eB8G8R8A8Unorm) &&
        format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return format;
    }
  }
  VALIDATION_LOG << "Picking a sub-optimal surface format.";
  return formats[0];
}

static vk::PresentModeKHR PickPresentationMode(
    std::vector<vk::PresentModeKHR> modes) {
  for (const auto& mode : modes) {
    if (mode == vk::PresentModeKHR::eMailbox) {
      return mode;
    }
  }

  VALIDATION_LOG << "Picking a sub-optimal presentation mode.";

  // Vulkan spec dictates that FIFO is always available.
  return vk::PresentModeKHR::eFifo;
}

static vk::Extent2D PickExtent(vk::SurfaceCapabilitiesKHR capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  vk::Extent2D actual_extent = {
      std::max(capabilities.minImageExtent.width,
               std::min(capabilities.maxImageExtent.width,
                        capabilities.currentExtent.width)),
      std::max(capabilities.minImageExtent.height,
               std::min(capabilities.maxImageExtent.height,
                        capabilities.currentExtent.height))};
  return actual_extent;
}

std::shared_ptr<SwapchainVK> SwapchainVK::Create(vk::Device device,
                                                 vk::SurfaceKHR surface,
                                                 SwapchainDetailsVK& details) {
  vk::SurfaceFormatKHR surface_format =
      PickSurfaceFormat(details.surface_formats_);
  vk::PresentModeKHR present_mode =
      PickPresentationMode(details.present_modes_);
  vk::Extent2D extent = PickExtent(details.surface_capabilities_);

  vk::SwapchainCreateInfoKHR create_info;
  create_info.surface = surface;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.presentMode = present_mode;
  create_info.imageExtent = extent;

  uint32_t image_count = details.surface_capabilities_.minImageCount;
  create_info.minImageCount = image_count + 1;  // for triple buffering
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
  create_info.preTransform = details.surface_capabilities_.currentTransform;
  create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  create_info.clipped = VK_TRUE;

  create_info.imageSharingMode = vk::SharingMode::eExclusive;
  create_info.queueFamilyIndexCount = 0;
  create_info.pQueueFamilyIndices = nullptr;

  create_info.oldSwapchain = nullptr;

  auto swapchain_res = device.createSwapchainKHRUnique(create_info);
  if (swapchain_res.result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to create swapchain: "
                   << vk::to_string(swapchain_res.result);
    return nullptr;
  }

  auto swapchain = std::make_shared<SwapchainVK>(std::move(swapchain_res.value),
                                                 surface_format.format, extent);
  swapchain->InitializeSwapchainImages(device);

  return swapchain;
}

SwapchainVK::~SwapchainVK() = default;

static vk::UniqueImageView CreateImageView(vk::Device device,
                                           vk::Image image,
                                           vk::Format image_format,
                                           vk::ImageAspectFlags aspect_flags) {
  vk::ImageViewCreateInfo create_info;
  create_info.image = image;
  create_info.viewType = vk::ImageViewType::e2D;
  create_info.format = image_format;

  create_info.components.r = vk::ComponentSwizzle::eIdentity;
  create_info.components.g = vk::ComponentSwizzle::eIdentity;
  create_info.components.b = vk::ComponentSwizzle::eIdentity;
  create_info.components.a = vk::ComponentSwizzle::eIdentity;

  create_info.subresourceRange.aspectMask = aspect_flags;
  create_info.subresourceRange.baseMipLevel = 0;
  create_info.subresourceRange.levelCount = 1;
  create_info.subresourceRange.baseArrayLayer = 0;
  create_info.subresourceRange.layerCount = 1;

  auto img_view_res = device.createImageViewUnique(create_info);
  if (img_view_res.result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to create image view: "
                   << vk::to_string(img_view_res.result);
    return vk::UniqueImageView(nullptr);
  }

  return std::move(img_view_res.value);
}

void SwapchainVK::InitializeSwapchainImages(vk::Device device) {
  auto res = device.getSwapchainImagesKHR(*swapchain_);
  if (res.result != vk::Result::eSuccess) {
    FML_CHECK(false) << "Failed to get swapchain images: "
                     << vk::to_string(res.result);
    return;
  }

  std::vector<vk::Image> images = res.value;
  for (const auto& image : images) {
    auto img_view = CreateImageView(device, image, image_format_,
                                    vk::ImageAspectFlagBits::eColor);
    if (!img_view) {
      FML_CHECK(false) << "Failed to create image view.";
      return;
    }
    swapchain_images_.push_back(
        std::make_unique<SwapchainImage>(image, std::move(img_view)));
  }
}

SwapchainVK::SwapchainVK(vk::UniqueSwapchainKHR swapchain,
                         vk::Format image_format,
                         vk::Extent2D extent)
    : swapchain_(std::move(swapchain)),
      image_format_(image_format),
      extent_(extent) {}

SwapchainImage::SwapchainImage(vk::Image image, vk::UniqueImageView image_view)
    : image_(image), image_view_(std::move(image_view)) {}

SwapchainImage::~SwapchainImage() = default;

}  // namespace impeller
