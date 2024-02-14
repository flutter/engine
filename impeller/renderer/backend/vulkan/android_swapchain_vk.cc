// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/android_swapchain_vk.h"
#include <memory>
#include "impeller/renderer/backend/vulkan/swapchain_impl_vk.h"

namespace impeller {

std::shared_ptr<SwapchainImplVK> AndroidSwapchainVK::Create(
    ANativeWindow* window,
    const std::shared_ptr<Context>& context,
    ISize size) {
  return std::shared_ptr<SwapchainImplVK>(
      new AndroidSwapchainVK(window, context, size));
}

AndroidSwapchainVK::AndroidSwapchainVK(ANativeWindow* window,
                                       const std::shared_ptr<Context>& context,
                                       ISize size)
    : SwapchainImplVK(context, size), window_(window) {
  ANativeWindow_acquire(window_);
}

AndroidSwapchainVK::~AndroidSwapchainVK() {
  ANativeWindow_release(window_);
}

bool AndroidSwapchainVK::IsValid() const {
  return true;
}

SwapchainImplVK::AcquireResult AndroidSwapchainVK::AcquireNextDrawable() {
  SwapchainImplVK::AcquireResult result;
  return result;
}

vk::Format AndroidSwapchainVK::GetSurfaceFormat() const {
  // TODO
  return vk::Format::eUndefined;
}

std::shared_ptr<SwapchainImplVK> AndroidSwapchainVK::RecreateSwapchain() {
  WaitIdle();
  return AndroidSwapchainVK::Create(window_, GetContext(), GetSize());
}

}  // namespace impeller
