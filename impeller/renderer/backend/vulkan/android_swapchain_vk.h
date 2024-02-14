// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_ANDROID_SWAPCHAIN_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_ANDROID_SWAPCHAIN_VK_H_

#include "impeller/renderer/backend/vulkan/swapchain_impl_vk.h"

#include <android/native_window.h>

namespace impeller {

class AndroidSwapchainVK : public SwapchainImplVK {
 public:
  static std::shared_ptr<SwapchainImplVK> Create(
      ANativeWindow* window,
      const std::shared_ptr<Context>& context,
      ISize size);
  ~AndroidSwapchainVK() override;

  bool IsValid() const override;
  AcquireResult AcquireNextDrawable() override;
  vk::Format GetSurfaceFormat() const override;
  std::shared_ptr<SwapchainImplVK> RecreateSwapchain() override;

 private:
  ANativeWindow* window_;

  explicit AndroidSwapchainVK(ANativeWindow* window,
                              const std::shared_ptr<Context>& context,
                              ISize size);

  AndroidSwapchainVK(const AndroidSwapchainVK&) = delete;

  AndroidSwapchainVK& operator=(const AndroidSwapchainVK&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_ANDROID_SWAPCHAIN_VK_H_
