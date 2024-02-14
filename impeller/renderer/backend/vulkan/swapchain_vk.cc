// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/swapchain_vk.h"

#include "flutter/fml/trace_event.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/vulkan/swapchain_impl_vk.h"
#if FML_OS_ANDROID
#include "impeller/renderer/backend/vulkan/android_swapchain_vk.h"
#endif  // FML_OS_ANDROID

namespace impeller {

#if FML_OS_ANDROID
std::shared_ptr<SwapchainVK> SwapchainVK::Create(
    const std::shared_ptr<Context>& context,
    ANativeWindow* window) {
  ISize size{ANativeWindow_getWidth(window), ANativeWindow_getHeight(window)};

  auto impl = AndroidSwapchainVK::Create(window, context, size);
  if (!impl || !impl->IsValid()) {
    VALIDATION_LOG << "Failed to create Android SwapchainVK implementation.";
    return nullptr;
  }
  return std::shared_ptr<SwapchainVK>(
      new SwapchainVK(std::move(impl), size, true));
}
#endif  // FML_OS_ANDROID

std::shared_ptr<SwapchainVK> SwapchainVK::Create(
    const std::shared_ptr<Context>& context,
    vk::UniqueSurfaceKHR surface,
    const ISize& size,
    bool enable_msaa) {
  auto impl =
      SwapchainImplVK::Create(context, std::move(surface), size, enable_msaa);
  if (!impl || !impl->IsValid()) {
    VALIDATION_LOG << "Failed to create SwapchainVK implementation.";
    return nullptr;
  }
  return std::shared_ptr<SwapchainVK>(
      new SwapchainVK(std::move(impl), size, enable_msaa));
}

SwapchainVK::SwapchainVK(std::shared_ptr<SwapchainImplVK> impl,
                         const ISize& size,
                         bool enable_msaa)
    : impl_(std::move(impl)), size_(size), enable_msaa_(enable_msaa) {}

SwapchainVK::~SwapchainVK() = default;

bool SwapchainVK::IsValid() const {
  return impl_ ? impl_->IsValid() : false;
}

void SwapchainVK::UpdateSurfaceSize(const ISize& size) {
  // Update the size of the swapchain. On the next acquired drawable,
  // the sizes may no longer match, forcing the swapchain to be recreated.
  size_ = size;
}

std::unique_ptr<Surface> SwapchainVK::AcquireNextDrawable() {
  if (!IsValid()) {
    return nullptr;
  }

  TRACE_EVENT0("impeller", __FUNCTION__);

  auto result = impl_->AcquireNextDrawable();
  if (!result.out_of_date && size_ == impl_->GetSize()) {
    return std::move(result.surface);
  }

  TRACE_EVENT0("impeller", "RecreateSwapchain");

  // This swapchain implementation indicates that it is out of date. Tear it
  // down and make a new one.
  auto new_impl = impl_->RecreateSwapchain();

  if (!new_impl || !new_impl->IsValid()) {
    VALIDATION_LOG << "Could not update swapchain.";
    // The old swapchain is dead because we took its surface. This is
    // unrecoverable.
    impl_.reset();
    return nullptr;
  }
  impl_ = std::move(new_impl);

  //----------------------------------------------------------------------------
  /// We managed to recreate the swapchain in the new configuration. Try again.
  ///
  return AcquireNextDrawable();
}

vk::Format SwapchainVK::GetSurfaceFormat() const {
  return IsValid() ? impl_->GetSurfaceFormat() : vk::Format::eUndefined;
}

}  // namespace impeller
