// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/android_swapchain_vk.h"

#include <memory>

#include "fml/platform/android/ndk_helpers.h"
#include "impeller/core/texture_descriptor.h"
#include "impeller/renderer/backend/vulkan/android_hardware_buffer_swapchain_image_vk.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/gpu_tracer_vk.h"
#include "impeller/renderer/backend/vulkan/surface_vk.h"
#include "impeller/renderer/backend/vulkan/swapchain_impl_vk.h"
#include "shell/platform/android/ndk_helpers.h"

namespace impeller {

std::shared_ptr<SwapchainImplVK> AndroidSwapchainVK::Create(
    ANativeWindow* window,
    const std::shared_ptr<Context>& context,
    ISize size,
    size_t image_count) {
  return std::shared_ptr<SwapchainImplVK>(
      new AndroidSwapchainVK(window, context, size, image_count));
}

AndroidSwapchainVK::AndroidSwapchainVK(ANativeWindow* window,
                                       const std::shared_ptr<Context>& context,
                                       ISize size,
                                       size_t image_count)
    : SwapchainImplVK(context, size),
      window_(window),
      image_count_(image_count) {
  ANativeWindow_acquire(window_);

  surface_control_ = flutter::NDKHelpers::ASurfaceControl_createFromWindow(
      window_, "Flutter Swapchain");

  auto& vk_context = ContextVK::Cast(*context);

  TextureDescriptor desc{
      .storage_mode = StorageMode::kDevicePrivate,
      .format = PixelFormat::kR8G8B8A8UNormInt,
      .size = size,
      .usage = static_cast<decltype(desc.usage)>(TextureUsage::kRenderTarget),
  };

  images_ = std::vector<std::shared_ptr<AndroidHardwareBufferSwapchainImageVK>>{
      AndroidHardwareBufferSwapchainImageVK::Create(desc,
                                                    vk_context.GetDevice()),
      AndroidHardwareBufferSwapchainImageVK::Create(desc,
                                                    vk_context.GetDevice()),
      AndroidHardwareBufferSwapchainImageVK::Create(desc,
                                                    vk_context.GetDevice()),
  };

  std::lock_guard lock(transaction_mutex_);
  for (size_t i = 0; i < images_.size(); i++) {
    const auto& image = images_[i];
    if (!image->IsValid()) {
      VALIDATION_LOG << "Could not create swapchain image.";
      return;
    }

    ContextVK::SetDebugName(vk_context.GetDevice(), image->GetImage(),
                            "SwapchainImage" + std::to_string(i));
    ContextVK::SetDebugName(vk_context.GetDevice(), image->GetImageView(),
                            "SwapchainImageView" + std::to_string(i));

    ASurfaceTransaction* transaction =
        flutter::NDKHelpers::ASurfaceTransaction_create();
    flutter::NDKHelpers::ASurfaceTransaction_setBuffer(
        transaction, surface_control_, image->GetHardwareBuffer(), -1);
    transactions_.push_back(transaction);
  }

  is_valid_ = true;
}

AndroidSwapchainVK::~AndroidSwapchainVK() {
  {
    std::lock_guard lock(transaction_mutex_);
    for (const auto& transaction : transactions_) {
      flutter::NDKHelpers::ASurfaceTransaction_delete(transaction);
    }
  }
  flutter::NDKHelpers::ASurfaceControl_release(surface_control_);
  ANativeWindow_release(window_);
}

bool AndroidSwapchainVK::IsValid() const {
  return is_valid_;
}

SwapchainImplVK::AcquireResult AndroidSwapchainVK::AcquireNextDrawable() {
  auto context = context_.lock();
  SwapchainImplVK::AcquireResult result;
  if (!context) {
    return result;
  }

  const auto& context_vk = ContextVK::Cast(*context);

  uint32_t current_frame = (current_frame_ + 1u) % images_.size();

  context_vk.GetGPUTracer()->MarkFrameStart();
  auto image = images_[current_frame];
  result.surface = SurfaceVK::WrapSwapchainImage(
      /*context=*/
      context,
      /*image=*/image,
      /*swap_callback=*/
      [weak_swapchain = weak_from_this(), image, current_frame]() -> bool {
        auto swapchain = weak_swapchain.lock();
        if (!swapchain) {
          return false;
        }
        return swapchain->Present(current_frame);
      },
      /*enable_msaa=*/true);

  return result;
}

bool AndroidSwapchainVK::Present(uint32_t image_index) {
  auto context = context_.lock();
  if (!context) {
    return false;
  }
  return false;
  // const auto& context_vk = ContextVK::Cast

  //                              context.GetGPUTracer()
  //                                  ->MarkFrameEnd();
  // flutter::NDKHelpers::ASurfaceTransaction_apply()
}

vk::Format AndroidSwapchainVK::GetSurfaceFormat() const {
  return surface_format_;
}

std::shared_ptr<SwapchainImplVK> AndroidSwapchainVK::RecreateSwapchain() {
  WaitIdle();
  return AndroidSwapchainVK::Create(window_, GetContext(), GetSize());
}

}  // namespace impeller
