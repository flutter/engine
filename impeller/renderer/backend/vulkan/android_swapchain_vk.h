// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_ANDROID_SWAPCHAIN_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_ANDROID_SWAPCHAIN_VK_H_

#include "impeller/renderer/backend/vulkan/android_hardware_buffer_swapchain_image_vk.h"
#include "impeller/renderer/backend/vulkan/swapchain_impl_vk.h"

#include <android/native_window.h>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace impeller {

class AndroidSwapchainVK
    : public SwapchainImplVK,
      public std::enable_shared_from_this<AndroidSwapchainVK> {
 public:
  static std::shared_ptr<SwapchainImplVK> Create(
      ANativeWindow* window,
      const std::shared_ptr<Context>& context,
      ISize size,
      size_t image_count = 3);
  ~AndroidSwapchainVK() override;

  bool IsValid() const override;
  AcquireResult AcquireNextDrawable() override;
  vk::Format GetSurfaceFormat() const override;
  std::shared_ptr<SwapchainImplVK> RecreateSwapchain() override;
  bool Present(uint32_t image_index);

 private:
  ANativeWindow* window_;
  ASurfaceControl* surface_control_;
  std::mutex transaction_mutex_;
  std::condition_variable IPLR_GUARDED_BY(transaction_mutex_) cv_;
  std::vector<ASurfaceTransaction*> IPLR_GUARDED_BY(transaction_mutex_)
      transactions_;
  vk::Format surface_format_ = vk::Format::eR8G8B8A8Unorm;
  std::vector<std::shared_ptr<AndroidHardwareBufferSwapchainImageVK>> images_;
  size_t current_frame_ = 0u;
  size_t image_count_ = 3u;
  bool is_valid_ = false;

  AndroidSwapchainVK(ANativeWindow* window,
                     const std::shared_ptr<Context>& context,
                     ISize size,
                     size_t image_count);

  AndroidSwapchainVK(const AndroidSwapchainVK&) = delete;

  AndroidSwapchainVK& operator=(const AndroidSwapchainVK&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_ANDROID_SWAPCHAIN_VK_H_
