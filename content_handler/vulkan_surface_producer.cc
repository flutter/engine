// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/content_handler/vulkan_surface_producer.h"
#include <memory>
#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/skia/include/gpu/vk/GrVkTypes.h"
#include "third_party/skia/src/gpu/vk/GrVkUtil.h"

namespace flutter_runner {

VulkanSurfaceProducer::VulkanSurfaceProducer() {
  valid_ = Initialize();
  if (!valid_) {
    FTL_LOG(ERROR) << "VulkanSurfaceProducer failed to initialize";
  }
}

VulkanSurfaceProducer::~VulkanSurfaceProducer() = default;

bool VulkanSurfaceProducer::Initialize() {
  vk_ = ftl::MakeRefCounted<vulkan::VulkanProcTable>();

  std::vector<std::string> extensions = {VK_KHR_SURFACE_EXTENSION_NAME};
  application_ = std::make_unique<vulkan::VulkanApplication>(
      *vk_, "FlutterContentHandler", std::move(extensions));

  if (!application_->IsValid() || !vk_->AreInstanceProcsSetup()) {
    // Make certain the application instance was created and it setup the
    // instance proc table entries.
    FTL_LOG(ERROR) << "Instance proc addresses have not been setup.";
    return false;
  }

  // Create the device.

  logical_device_ = application_->AcquireFirstCompatibleLogicalDevice();

  if (logical_device_ == nullptr || !logical_device_->IsValid() ||
      !vk_->AreDeviceProcsSetup()) {
    // Make certain the device was created and it setup the device proc table
    // entries.
    FTL_LOG(ERROR) << "Device proc addresses have not been setup.";
    return false;
  }

  if (!vk_->HasAcquiredMandatoryProcAddresses()) {
    FTL_LOG(ERROR) << "Failed to acquire mandatory proc addresses";
    return false;
  }

  if (!vk_->IsValid()) {
    FTL_LOG(ERROR) << "VulkanProcTable invalid";
    return false;
  }

  auto interface = vk_->CreateSkiaInterface();

  if (interface == nullptr || !interface->validate(0)) {
    FTL_LOG(ERROR) << "interface invalid";
    return false;
  }

  uint32_t skia_features = 0;
  if (!logical_device_->GetPhysicalDeviceFeaturesSkia(&skia_features)) {
    FTL_LOG(ERROR) << "Failed to get physical device features";

    return false;
  }

  backend_context_ = sk_make_sp<GrVkBackendContext>();
  backend_context_->fInstance = application_->GetInstance();
  backend_context_->fPhysicalDevice =
      logical_device_->GetPhysicalDeviceHandle();
  backend_context_->fDevice = logical_device_->GetHandle();
  backend_context_->fQueue = logical_device_->GetQueueHandle();
  backend_context_->fGraphicsQueueIndex =
      logical_device_->GetGraphicsQueueIndex();
  backend_context_->fMinAPIVersion = application_->GetAPIVersion();
  backend_context_->fFeatures = skia_features;
  backend_context_->fInterface.reset(interface.release());

  logical_device_->ReleaseDeviceOwnership();
  application_->ReleaseInstanceOwnership();

  context_.reset(GrContext::Create(
      kVulkan_GrBackend,
      reinterpret_cast<GrBackendContext>(backend_context_.get())));

  context_->setResourceCacheLimits(vulkan::kGrCacheMaxCount,
                                   vulkan::kGrCacheMaxByteSize);

  FTL_DLOG(INFO) << "Successfully initialized VulkanRasterizer";
  return true;
}

std::unique_ptr<VulkanSurfaceProducer::Surface>
VulkanSurfaceProducer::CreateSurface(mozart::client::Session* session,
                                     uint32_t width,
                                     uint32_t height) {
  VkResult vk_result;

  VkImageCreateInfo image_create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_B8G8R8A8_UNORM,
      .extent = VkExtent3D{width, height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VkImage vk_image;
  vk_result = VK_CALL_LOG_ERROR(vkCreateImage(
      backend_context_->fDevice, &image_create_info, nullptr, &vk_image));
  if (vk_result)
    return nullptr;

  VkMemoryRequirements memory_reqs;
  vkGetImageMemoryRequirements(backend_context_->fDevice, vk_image,
                               &memory_reqs);

  uint32_t memory_type = 0;
  for (; memory_type < 32; memory_type++) {
    if ((memory_reqs.memoryTypeBits & (1 << memory_type)))
      break;
  }

  VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memory_reqs.size,
      .memoryTypeIndex = memory_type,
  };

  VkDeviceMemory vk_memory;
  vk_result = VK_CALL_LOG_ERROR(vkAllocateMemory(
      backend_context_->fDevice, &alloc_info, NULL, &vk_memory));
  if (vk_result)
    return nullptr;

  vk_result = VK_CALL_LOG_ERROR(
      vkBindImageMemory(backend_context_->fDevice, vk_image, vk_memory, 0));
  if (vk_result)
    return nullptr;

  const GrVkImageInfo image_info = {
      .fImage = vk_image,
      .fAlloc = {vk_memory, 0, memory_reqs.size, 0},
      .fImageTiling = image_create_info.tiling,
      .fImageLayout = image_create_info.initialLayout,
      .fFormat = image_create_info.format,
      .fLevelCount = image_create_info.mipLevels,
  };

  GrBackendRenderTargetDesc desc;
  desc.fWidth = width;
  desc.fHeight = height;
  desc.fConfig = kSBGRA_8888_GrPixelConfig;
  desc.fOrigin = kTopLeft_GrSurfaceOrigin;
  desc.fSampleCnt = 0;
  desc.fStencilBits = 0;

  desc.fRenderTargetHandle = reinterpret_cast<GrBackendObject>(&image_info);

  SkSurfaceProps props(SkSurfaceProps::InitType::kLegacyFontHost_InitType);

  auto sk_surface = SkSurface::MakeFromBackendRenderTarget(context_.get(), desc,
                                                           nullptr, &props);
  if (!sk_surface) {
    FTL_LOG(ERROR) << "MakeFromBackendRenderTarget Failed";
    return nullptr;
  }

  uint32_t vmo_handle;
  vk_result = VK_CALL_LOG_ERROR(vkExportDeviceMemoryMAGMA(
      backend_context_->fDevice, vk_memory, &vmo_handle));
  if (vk_result)
    return nullptr;

  mx::vmo vmo(vmo_handle);

  size_t vmo_size;
  vmo.get_size(&vmo_size);

  FTL_DCHECK(vmo_size >= memory_reqs.size);

  if (!sk_surface || sk_surface->getCanvas() == nullptr) {
    FTL_LOG(ERROR) << "surface invalid";
    return nullptr;
  }

  return std::make_unique<Surface>(backend_context_,  //
                                   sk_surface,        //
                                   std::move(vmo),    //
                                   vk_image,          //
                                   vk_memory,         //
                                   session            //
                                   );
}

sk_sp<SkSurface> VulkanSurfaceProducer::ProduceSurface(
    SkISize size,
    mozart::client::Session* session,
    uint32_t& session_image_id,
    mx::event& acquire_release,
    mx::event& release_fence) {
  if (size.isEmpty()) {
    FTL_LOG(ERROR) << "Attempting to create surface with empty size";
    return nullptr;
  }

  // These casts are safe because of the early out on frame_size.isEmpty()
  auto width = static_cast<uint32_t>(size.width());
  auto height = static_cast<uint32_t>(size.height());

  std::unique_ptr<Surface> surface;
  // Try and find a Swapchain with surfaces of the right size
  auto it = available_surfaces_.find(MakeSizeKey(width, height));
  if (it == available_surfaces_.end()) {
    // No matching Swapchain exists, create a new surfaces
    surface = CreateSurface(session, width, height);
  } else {
    auto& swapchain = it->second;
    if (swapchain.queue.size() == 0) {
      // matching Swapchain exists, but does not have any buffers available in
      // it
      surface = CreateSurface(session, width, height);
    } else {
      surface = std::move(swapchain.queue.front());
      swapchain.queue.pop();
      swapchain.tick_count = 0;
      surface->sk_surface->getCanvas()->restoreToCount(1);
    }
  }

  if (!surface) {
    FTL_LOG(ERROR) << "Failed to produce surface.";
    return nullptr;
  }

  session_image_id = surface->session_image->id();
  // TODO: Acquire fence.
  // TODO: Release fence.

  auto sk_surface = surface->sk_surface;
  sk_surface->getCanvas()->save();

  PendingSurfaceInfo info;
  info.handler_key = dummy_handler_key_++;
  info.surface = std::move(surface);
  outstanding_surfaces_.push_back(std::move(info));

  return sk_surface;
}

bool VulkanSurfaceProducer::FinishFrame() {
  // Finish Rendering.
  context_->flush();
  VkResult result =
      VK_CALL_LOG_ERROR(vkQueueWaitIdle(backend_context_->fQueue));

  if (result) {
    return false;
  }

  for (auto& info : outstanding_surfaces_) {
    auto dummy_key = info.handler_key;
    pending_surfaces_.insert(std::make_pair(dummy_key, std::move(info)));
  }
  outstanding_surfaces_.clear();
  return true;
}

void VulkanSurfaceProducer::Tick() {
  // We are never going to get this from the compositor.
  RecycleBuffers();

  for (auto it = available_surfaces_.begin();
       it != available_surfaces_.end();) {
    auto& swapchain = it->second;
    swapchain.tick_count++;
    if (swapchain.tick_count > Swapchain::kMaxTickBeforeDiscard) {
      it = available_surfaces_.erase(it);
    } else {
      it++;
    }
  }
}

void VulkanSurfaceProducer::RecycleBuffers() {
  for (auto& it : pending_surfaces_) {
    // Add the newly available buffer to the swapchain.
    PendingSurfaceInfo& info = it.second;

    // try and find a Swapchain with surfaces of the right size
    size_key_t key = MakeSizeKey(info.surface->sk_surface->width(),
                                 info.surface->sk_surface->height());
    auto swapchain_it = available_surfaces_.find(key);
    if (swapchain_it == available_surfaces_.end()) {
      // No matching Swapchain exists, create one
      Swapchain swapchain;
      if (swapchain.queue.size() + 1 <= Swapchain::kMaxSurfaces) {
        swapchain.queue.push(std::move(info.surface));
      }
      available_surfaces_.insert(std::make_pair(key, std::move(swapchain)));
    } else {
      auto& swapchain = swapchain_it->second;
      if (swapchain.queue.size() + 1 <= Swapchain::kMaxSurfaces) {
        swapchain.queue.push(std::move(info.surface));
      }
    }
  }
  pending_surfaces_.clear();
}

}  // namespace flutter_runner
