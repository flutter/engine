// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/vulkan/vulkan_backbuffer.h"
#include "flutter/vulkan/vulkan_device.h"
#include "flutter/vulkan/vulkan_image.h"
#include "flutter/vulkan/vulkan_proc_table.h"
#include "flutter/vulkan/vulkan_surface.h"
#include "flutter/vulkan/vulkan_swapchain.h"
#include "third_party/skia/include/gpu/vk/GrVkTypes.h"
#include "third_party/skia/src/gpu/vk/GrVkUtil.h"

namespace vulkan {

VulkanSwapchain::VulkanSwapchain(const VulkanProcTable& p_vk,
                                 const VulkanDevice& device,
                                 const VulkanSurface& surface,
                                 GrContext* skia_context,
                                 std::unique_ptr<VulkanSwapchain> old_swapchain)
    : vk(p_vk),
      device_(device),
      capabilities_(),
      surface_format_(),
      present_mode_(),
      current_pipeline_stage_(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
      current_backbuffer_index_(0),
      current_image_index_(0),
      valid_(false) {
  if (!device_.IsValid() || !surface.IsValid() || skia_context == nullptr) {
    return;
  }

  if (!device_.GetSurfaceCapabilities(surface, &capabilities_)) {
    return;
  }

  if (!device_.ChooseSurfaceFormat(surface, &surface_format_)) {
    return;
  }

  if (!device_.ChoosePresentMode(surface, &present_mode_)) {
    return;
  }

  // Construct the Swapchain

  VkSwapchainKHR old_swapchain_handle = VK_NULL_HANDLE;

  if (old_swapchain != nullptr && old_swapchain->IsValid()) {
    old_swapchain_handle = old_swapchain->swapchain_;
    // The unique pointer to the swapchain will go out of scope here
    // and its handle collected after the appropriate device wait.
  }

  const VkSwapchainCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .surface = surface.Handle(),
      .minImageCount = capabilities_.minImageCount,
      .imageFormat = surface_format_.format,
      .imageColorSpace = surface_format_.colorSpace,
      .imageExtent = capabilities_.currentExtent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = present_mode_,
      .clipped = VK_FALSE,
      .oldSwapchain = old_swapchain_handle,
  };

  VkSwapchainKHR swapchain = VK_NULL_HANDLE;

  if (vk.createSwapchainKHR(device_.Handle(), &create_info, nullptr,
                            &swapchain) != VK_SUCCESS) {
    return;
  }

  swapchain_ = {swapchain, [this](VkSwapchainKHR swapchain) {
                  FTL_ALLOW_UNUSED_LOCAL(device_.WaitIdle());
                  vk.destroySwapchainKHR(device_.Handle(), swapchain, nullptr);
                }};

  if (!CreateSwapchainImages(skia_context)) {
    return;
  }

  valid_ = true;
}

VulkanSwapchain::~VulkanSwapchain() = default;

bool VulkanSwapchain::IsValid() const {
  return valid_;
}

std::vector<VkImage> VulkanSwapchain::GetImages() const {
  uint32_t count = 0;
  if (vk.getSwapchainImagesKHR(device_.Handle(), swapchain_, &count, nullptr) !=
      VK_SUCCESS) {
    return {};
  }

  if (count == 0) {
    return {};
  }

  std::vector<VkImage> images;

  images.resize(count);

  if (vk.getSwapchainImagesKHR(device_.Handle(), swapchain_, &count,
                               images.data()) != VK_SUCCESS) {
    return {};
  }

  return images;
}

SkISize VulkanSwapchain::GetSize() const {
  VkExtent2D extents = capabilities_.currentExtent;

  if (extents.width < capabilities_.minImageExtent.width) {
    extents.width = capabilities_.minImageExtent.width;
  } else if (extents.width > capabilities_.maxImageExtent.width) {
    extents.width = capabilities_.maxImageExtent.width;
  }

  if (extents.height < capabilities_.minImageExtent.height) {
    extents.height = capabilities_.minImageExtent.height;
  } else if (extents.height > capabilities_.maxImageExtent.height) {
    extents.height = capabilities_.maxImageExtent.height;
  }

  return SkISize::Make(extents.width, extents.height);
}

sk_sp<SkSurface> VulkanSwapchain::CreateSkiaSurface(GrContext* gr_context,
                                                    VkImage image,
                                                    const SkISize& size) const {
  if (gr_context == nullptr) {
    return nullptr;
  }

  GrPixelConfig pixel_config = kUnknown_GrPixelConfig;

  if (!GrVkFormatToPixelConfig(surface_format_.format, &pixel_config)) {
    // Vulkan format unsupported by Skia.
    return nullptr;
  }

  const GrVkImageInfo image_info = {
      .fImage = image,
      .fAlloc = {VK_NULL_HANDLE, 0, 0, 0},
      .fImageTiling = VK_IMAGE_TILING_OPTIMAL,
      .fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .fFormat = surface_format_.format,
      .fLevelCount = 1,
  };

  GrBackendRenderTargetDesc desc;
  desc.fWidth = size.fWidth;
  desc.fHeight = size.fHeight;
  desc.fConfig = pixel_config;
  desc.fOrigin = kTopLeft_GrSurfaceOrigin;
  desc.fSampleCnt = 0;
  desc.fStencilBits = 0;
  desc.fRenderTargetHandle = reinterpret_cast<GrBackendObject>(&image_info);

  SkSurfaceProps props(SkSurfaceProps::InitType::kLegacyFontHost_InitType);

  return SkSurface::MakeFromBackendRenderTarget(gr_context, desc, &props);
}

bool VulkanSwapchain::CreateSwapchainImages(GrContext* skia_context) {
  std::vector<VkImage> images = GetImages();

  if (images.size() == 0) {
    return false;
  }

  const SkISize surface_size = GetSize();

  for (const VkImage& image : images) {
    // Populate the backbuffer.
    auto backbuffer = std::make_unique<VulkanBackbuffer>(vk, device_.Handle(),
                                                         device_.CommandPool());

    if (!backbuffer->IsValid()) {
      return false;
    }

    backbuffers_.emplace_back(std::move(backbuffer));

    // Populate the image.
    auto vulkan_image = std::make_unique<VulkanImage>(image);

    if (!vulkan_image->IsValid()) {
      return false;
    }

    images_.emplace_back(std::move(vulkan_image));

    // Populate the surface.
    auto surface = CreateSkiaSurface(skia_context, image, surface_size);

    if (surface == nullptr) {
      return false;
    }

    surfaces_.emplace_back(std::move(surface));
  }

  FTL_DCHECK(backbuffers_.size() == images_.size());
  FTL_DCHECK(images_.size() == surfaces_.size());

  return true;
}

VulkanBackbuffer* VulkanSwapchain::NextBackbuffer() {
  auto available_backbuffers = backbuffers_.size();

  if (available_backbuffers == 0) {
    return nullptr;
  }

  auto next_backbuffer_index =
      (current_backbuffer_index_ + 1) % backbuffers_.size();

  auto& backbuffer = backbuffers_[next_backbuffer_index];

  if (!backbuffer->IsValid()) {
    return nullptr;
  }

  current_backbuffer_index_ = next_backbuffer_index;
  return backbuffer.get();
}

VulkanSwapchain::AcquireResult VulkanSwapchain::AcquireSurface() {
  AcquireResult error = {AcquireType::ErrorSurfaceLost, nullptr};

  if (!IsValid()) {
    return error;
  }

  // ---------------------------------------------------------------------------
  // Step 0:
  // Acquire the next available backbuffer.
  // ---------------------------------------------------------------------------
  auto backbuffer = NextBackbuffer();

  if (backbuffer == nullptr) {
    return error;
  }

  // ---------------------------------------------------------------------------
  // Step 1:
  // Wait for use readiness.
  // ---------------------------------------------------------------------------
  if (!backbuffer->WaitFences()) {
    return error;
  }

  // ---------------------------------------------------------------------------
  // Step 2:
  // Put semaphores in unsignled state.
  // ---------------------------------------------------------------------------
  if (!backbuffer->ResetFences()) {
    return error;
  }

  // ---------------------------------------------------------------------------
  // Step 3:
  // Acquire the next image index.
  // ---------------------------------------------------------------------------
  uint32_t next_image_index = 0;

  switch (vk.acquireNextImageKHR(
      device_.Handle(), swapchain_, std::numeric_limits<uint64_t>::max(),
      backbuffer->UsageSemaphore(), VK_NULL_HANDLE, &next_image_index)) {
    case VK_SUCCESS:
      break;
    case VK_ERROR_OUT_OF_DATE_KHR:
      return {AcquireType::ErrorSurfaceOutOfDate, nullptr};
    default:
      // Includes VK_ERROR_SURFACE_LOST_KHR.
      return {AcquireType::ErrorSurfaceLost, nullptr};
  }

  // Simple sanity checking of image index.
  if (next_image_index >= images_.size()) {
    return error;
  }

  auto& image = images_[next_image_index];
  if (!image->IsValid()) {
    return error;
  }

  // ---------------------------------------------------------------------------
  // Step 4:
  // Start recording to the command buffer.
  // ---------------------------------------------------------------------------
  if (!backbuffer->UsageCommandBuffer().Begin()) {
    return error;
  }

  // ---------------------------------------------------------------------------
  // Step 5:
  // Set image layout to color attachment mode.
  // ---------------------------------------------------------------------------
  VkPipelineStageFlagBits destination_pipline_stage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkImageLayout destination_image_layout =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  if (!image->InsertImageMemoryBarrier(
          backbuffer->UsageCommandBuffer(),      // command buffer
          current_pipeline_stage_,               // src_pipline_bits
          destination_pipline_stage,             // dest_pipline_bits
          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,  // dest_access_flags
          destination_image_layout               // dest_layout
          )) {
    return error;
  } else {
    current_pipeline_stage_ = destination_pipline_stage;
  }

  // ---------------------------------------------------------------------------
  // Step 6:
  // End recording to the command buffer.
  // ---------------------------------------------------------------------------
  if (!backbuffer->UsageCommandBuffer().End()) {
    return error;
  }

  // ---------------------------------------------------------------------------
  // Step 7:
  // Submit the command buffer to the device queue.
  // ---------------------------------------------------------------------------
  std::vector<VkSemaphore> wait_semaphores = {backbuffer->UsageSemaphore()};
  std::vector<VkSemaphore> signal_semaphores = {};
  std::vector<VkCommandBuffer> command_buffers = {
      backbuffer->UsageCommandBuffer().Handle()};

  if (!device_.QueueSubmit(
          {destination_pipline_stage},  // wait_dest_pipeline_stages
          wait_semaphores,              // wait_semaphores
          signal_semaphores,            // signal_semaphores
          command_buffers,              // command_buffers
          backbuffer->UsageFence()      // fence
          )) {
    return error;
  }

  // ---------------------------------------------------------------------------
  // Step 8:
  // Tell Skia about the updated image layout.
  // ---------------------------------------------------------------------------
  sk_sp<SkSurface> surface = surfaces_[next_image_index];

  if (surface == nullptr) {
    return error;
  }

  GrVkImageInfo* image_info = nullptr;
  if (!surface->getRenderTargetHandle(
          reinterpret_cast<GrBackendObject*>(&image_info),
          SkSurface::kFlushRead_BackendHandleAccess)) {
    return error;
  }

  image_info->updateImageLayout(destination_image_layout);
  current_image_index_ = next_image_index;

  return {AcquireType::Success, surface};
}

bool VulkanSwapchain::Submit() {
  if (!IsValid()) {
    return false;
  }

  sk_sp<SkSurface> surface = surfaces_[current_image_index_];
  const std::unique_ptr<VulkanImage>& image = images_[current_image_index_];
  auto backbuffer = backbuffers_[current_backbuffer_index_].get();

  // ---------------------------------------------------------------------------
  // Step 0:
  // Notify to Skia that we will read from its backend object.
  // ---------------------------------------------------------------------------
  GrVkImageInfo* image_info = nullptr;
  if (!surface->getRenderTargetHandle(
          reinterpret_cast<GrBackendObject*>(&image_info),
          SkSurface::kFlushRead_BackendHandleAccess)) {
    return false;
  }

  // ---------------------------------------------------------------------------
  // Step 1:
  // Start recording to the command buffer.
  // ---------------------------------------------------------------------------
  if (!backbuffer->RenderCommandBuffer().Begin()) {
    return false;
  }

  // ---------------------------------------------------------------------------
  // Step 2:
  // Set image layout to present mode.
  // ---------------------------------------------------------------------------
  VkPipelineStageFlagBits destination_pipline_stage =
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  VkImageLayout destination_image_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  if (!image->InsertImageMemoryBarrier(
          backbuffer->RenderCommandBuffer(),  // command buffer
          current_pipeline_stage_,            // src_pipline_bits
          destination_pipline_stage,          // dest_pipline_bits
          VK_ACCESS_MEMORY_READ_BIT,          // dest_access_flags
          destination_image_layout            // dest_layout
          )) {
    return false;
  } else {
    current_pipeline_stage_ = destination_pipline_stage;
  }

  // ---------------------------------------------------------------------------
  // Step 3:
  // End recording to the command buffer.
  // ---------------------------------------------------------------------------
  if (!backbuffer->RenderCommandBuffer().End()) {
    return false;
  }

  // ---------------------------------------------------------------------------
  // Step 4:
  // Submit the command buffer to the device queue. Tell it to signal the render
  // semaphore.
  // ---------------------------------------------------------------------------
  std::vector<VkSemaphore> wait_semaphores = {};
  std::vector<VkSemaphore> queue_signal_semaphores = {
      backbuffer->RenderSemaphore()};
  std::vector<VkCommandBuffer> command_buffers = {
      backbuffer->RenderCommandBuffer().Handle()};

  if (!device_.QueueSubmit(
          {/* Empty. No wait semaphores. */},  // wait_dest_pipeline_stages
          wait_semaphores,                     // wait_semaphores
          queue_signal_semaphores,             // signal_semaphores
          command_buffers,                     // command_buffers
          backbuffer->RenderFence()            // fence
          )) {
    return false;
  }

  // ---------------------------------------------------------------------------
  // Step 5:
  // Submit the present operation and wait on the render semaphore.
  // ---------------------------------------------------------------------------
  VkSwapchainKHR swapchain = swapchain_;
  uint32_t present_image_index = static_cast<uint32_t>(current_image_index_);
  const VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount =
          static_cast<uint32_t>(queue_signal_semaphores.size()),
      .pWaitSemaphores = queue_signal_semaphores.data(),
      .swapchainCount = 1,
      .pSwapchains = &swapchain,
      .pImageIndices = &present_image_index,
      .pResults = nullptr,
  };

  if (vk.queuePresentKHR(device_.QueueHandle(), &present_info) != VK_SUCCESS) {
    return false;
  }

  return true;
}

}  // namespace vulkan
