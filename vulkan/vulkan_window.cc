// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include "flutter/vulkan/vulkan_application.h"
#include "flutter/vulkan/vulkan_device.h"
#include "flutter/vulkan/vulkan_native_surface.h"
#include "flutter/vulkan/vulkan_surface.h"
#include "flutter/vulkan/vulkan_swapchain.h"
#include "flutter/vulkan/vulkan_window.h"
#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/skia/include/gpu/vk/GrVkInterface.h"

namespace vulkan {

VulkanWindow::VulkanWindow(std::unique_ptr<VulkanNativeSurface> native_surface)
    : valid_(false) {
  if (native_surface == nullptr || !native_surface->IsValid() ||
      !vk.IsValid()) {
    return;
  }

  // Create the application instance.

  std::vector<std::string> extensions = {
      VK_KHR_SURFACE_EXTENSION_NAME,   // parent extension
      native_surface->ExtensionName()  // child extension
  };

  application_ = std::make_unique<VulkanApplication>(vk, "Flutter", extensions);

  if (!application_->IsValid()) {
    return;
  }

  // Create the device.

  logical_device_ = application_->AcquireCompatibleLogicalDevice();

  if (logical_device_ == nullptr || !logical_device_->IsValid()) {
    return;
  }

  // Create the logical surface from the native platform surface.

  surface_ = std::make_unique<VulkanSurface>(vk, *application_,
                                             std::move(native_surface));

  if (!surface_->IsValid()) {
    return;
  }

  // Create the Skia GrContext.

  skia_gr_context_ = CreateSkiaGrContext();

  if (skia_gr_context_ == nullptr) {
    return;
  }

  // Create the swapchain.

  if (!RecreateSwapchain()) {
    return;
  }

  valid_ = true;
}

VulkanWindow::~VulkanWindow() = default;

bool VulkanWindow::IsValid() const {
  return valid_;
}

GrContext* VulkanWindow::SkiaGrContext() {
  return skia_gr_context_.get();
}

sk_sp<GrContext> VulkanWindow::CreateSkiaGrContext() const {
  auto backend_context = CreateSkiaBackendContext();

  if (backend_context == nullptr) {
    return nullptr;
  }

  sk_sp<GrContext> context;
  context.reset(GrContext::Create(
      kVulkan_GrBackend,
      reinterpret_cast<GrBackendContext>(backend_context.get())));
  return context;
}

sk_sp<const GrVkBackendContext> VulkanWindow::CreateSkiaBackendContext() const {
  auto interface = vk.SkiaInterface();

  if (interface == nullptr || !interface->validate()) {
    return nullptr;
  }

  uint32_t skia_features = 0;
  if (!logical_device_->GetPhysicalDeviceFeaturesSkia(&skia_features)) {
    return nullptr;
  }

  auto context = sk_make_sp<GrVkBackendContext>();
  context->fInstance = application_->Instance();
  context->fPhysicalDevice = logical_device_->PhysicalDeviceHandle();
  context->fDevice = logical_device_->Handle();
  context->fQueue = logical_device_->QueueHandle();
  context->fGraphicsQueueIndex = logical_device_->GraphicsQueueIndex();
  context->fMinAPIVersion = application_->APIVersion();
  context->fExtensions = kKHR_surface_GrVkExtensionFlag |
                         kKHR_swapchain_GrVkExtensionFlag |
                         surface_->NativeSurface().SkiaExtensionName();
  context->fFeatures = skia_features;
  context->fInterface.reset(interface.release());
  return context;
}

sk_sp<SkSurface> VulkanWindow::AcquireSurface() {
  if (!IsValid()) {
    return nullptr;
  }

  if (swapchain_->GetSize() != surface_->GetSize()) {
    FTL_DLOG(INFO) << "Swapchain and surface sizes are out of sync. Recreating "
                      "swapchain.";
    if (!RecreateSwapchain()) {
      valid_ = false;
      return nullptr;
    }
  }

  while (true) {
    sk_sp<SkSurface> surface;
    auto acquire_result = VulkanSwapchain::AcquireType::ErrorSurfaceLost;

    std::tie(acquire_result, surface) = swapchain_->AcquireSurface();

    if (acquire_result == VulkanSwapchain::AcquireType::Success) {
      // Successfully acquired a surface from the swapchain. Nothing more to do.
      return surface;
    }

    if (acquire_result == VulkanSwapchain::AcquireType::ErrorSurfaceLost) {
      // Surface is lost. This is an unrecoverable error.
      return nullptr;
    }

    if (acquire_result == VulkanSwapchain::AcquireType::ErrorSurfaceOutOfDate) {
      // Surface out of date. Recreate the swapchain at the new configuration.
      if (RecreateSwapchain()) {
        // Swapchain was recreated, try surface acquisition again.
        continue;
      } else {
        // Could not recreate the swapchain at the new configuration.
        valid_ = false;
        return nullptr;
      }
    }

    FTL_DCHECK(false) << "Unhandled VulkanSwapchain::AcquireResult";
    break;
  }

  return nullptr;
}

bool VulkanWindow::SwapBuffers() {
  if (!IsValid()) {
    return false;
  }

  return swapchain_->Submit();
}

bool VulkanWindow::RecreateSwapchain() {
  swapchain_.reset();

  if (!vk.IsValid()) {
    return false;
  }

  if (logical_device_ == nullptr || !logical_device_->IsValid()) {
    return false;
  }

  if (surface_ == nullptr || !surface_->IsValid()) {
    return false;
  }

  if (skia_gr_context_ == nullptr) {
    return false;
  }

  auto swapchain = std::make_unique<VulkanSwapchain>(
      vk, *logical_device_, *surface_, skia_gr_context_.get());

  if (!swapchain->IsValid()) {
    return false;
  }

  swapchain_ = std::move(swapchain);
  return true;
}

}  // namespace vulkan
