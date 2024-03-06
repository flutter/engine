// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/swapchain/ahb/ahb_swapchain_vk.h"

#include "flutter/fml/closure.h"
#include "flutter/fml/trace_event.h"
#include "impeller/renderer/backend/vulkan/swapchain/ahb/ahb_surface_vk.h"
#include "impeller/toolkit/android/surface_transaction.h"

namespace impeller {

static constexpr vk::Format ToVKFormat(android::HardwareBufferFormat format) {
  switch (format) {
    case android::HardwareBufferFormat::kR8G8B8A8UNormInt:
      return vk::Format::eR8G8B8A8Unorm;
  }
  FML_UNREACHABLE();
}

std::shared_ptr<AHBSwapchainVK> AHBSwapchainVK::Create(
    const std::shared_ptr<Context>& context,
    ANativeWindow* window,
    size_t max_drawable_count) {
  auto swapchain = std::shared_ptr<AHBSwapchainVK>(
      new AHBSwapchainVK(context, window, max_drawable_count));
  if (!swapchain->IsValid()) {
    VALIDATION_LOG << "Could not create valid AHB swapchain.";
    return nullptr;
  }
  return swapchain;
}

AHBSwapchainVK::AHBSwapchainVK(const std::shared_ptr<Context>& context,
                               ANativeWindow* window,
                               size_t max_drawable_count)
    : context_(context),
      native_window_(std::make_shared<android::NativeWindow>(window)),
      drawable_count_sema_(max_drawable_count) {
  if (!context || !drawable_count_sema_.IsValid()) {
    return;
  }

  if (!native_window_->IsValid()) {
    VALIDATION_LOG << "Invalid window when creating a swapchain.";
    return;
  }

  auto surface_control = std::make_shared<android::SurfaceControl>(
      native_window_->GetHandle(), "FlutterImpellerVulkan");

  if (!surface_control->IsValid()) {
    VALIDATION_LOG << "Could not create surface control from window.";
    return;
  }

  desc_ = android::HardwareBufferDescriptor::MakeForSwapchainImage(
      native_window_->GetSize());

  if (!desc_.IsAllocatable()) {
    VALIDATION_LOG << "Hardware buffer is not allocatable.";
    return;
  }

  ContextVK::Cast(*context).SetOffscreenFormat(
      ToPixelFormat(ToVKFormat(desc_.format)));

  surface_control_ = std::move(surface_control);
  is_valid_ = true;
}

AHBSwapchainVK::~AHBSwapchainVK() = default;

// |SwapchainVK|
bool AHBSwapchainVK::IsValid() const {
  return is_valid_;
}

// |SwapchainVK|
vk::Format AHBSwapchainVK::GetSurfaceFormat() const {
  Lock lock(mutex_);
  return ToVKFormat(desc_.format);
}

// |SwapchainVK|
void AHBSwapchainVK::UpdateSurfaceSize(const ISize& size) {
  TRACE_EVENT0("impeller", __FUNCTION__);

  Lock lock(mutex_);

  if (desc_.size == size) {
    return;
  }

  auto new_desc = desc_;
  new_desc.size = size;
  if (!new_desc.IsAllocatable()) {
    VALIDATION_LOG << "New surface size is not allocatable.";
    return;
  }

  desc_ = new_desc;
}

std::shared_ptr<AHBSurfaceVK> AHBSwapchainVK::CreateNewSurface() {
  TRACE_EVENT0("impeller", __FUNCTION__);

  auto ahb = std::make_unique<android::HardwareBuffer>(desc_);
  if (!ahb || !ahb->IsValid()) {
    VALIDATION_LOG << "Could not create hardware buffer.";
    return nullptr;
  }
  auto context = context_.lock();
  if (!context) {
    VALIDATION_LOG << "Context died during image acquisition.";
    return nullptr;
  }
  auto texture = std::make_shared<AHBTextureSourceVK>(context, std::move(ahb));
  if (!texture->IsValid()) {
    VALIDATION_LOG << "Could not wrap hardware buffer into a texture.";
    return nullptr;
  }
  auto surface = AHBSurfaceVK::WrapSwapchainImage(context, weak_from_this(),
                                                  std::move(texture));
  if (!surface->IsValid()) {
    VALIDATION_LOG << "Could not create surface with wrapped texture.";
    return nullptr;
  }
  return surface;
}

// |SwapchainVK|
std::shared_ptr<Surface> AHBSwapchainVK::AcquireNextDrawable() {
  TRACE_EVENT0("impeller", __FUNCTION__);

  {
    TRACE_EVENT0("impeller", "CompositorBackpressure");
    // Accompanying signal is in the call to recycle the surface after the
    // compositor is done using it.
    if (!drawable_count_sema_.Wait()) {
      VALIDATION_LOG << "Failed waiting for drawable acquisition.";
      return nullptr;
    }
  }

  Lock lock(mutex_);

  if (auto surface = PopRecyclable()) {
    FML_LOG(IMPORTANT) << "Recycled surface.";
    return surface;
  }

  if (auto surface = CreateNewSurface()) {
    FML_LOG(IMPORTANT) << "Created surface.";
    return surface;
  }

  return nullptr;
}

bool AHBSwapchainVK::PresentSurface(
    const std::shared_ptr<AHBSurfaceVK>& surface) {
  // We must signal the drawables semaphore in all cases. Otherwise, new
  // drawable acquisitions may wait forever. In case we fail before the
  // compositor even attempts to use the surface, recycle it.
  fml::ScopedCleanupClosure recycle_on_fail(
      [&]() { OnSurfaceDidCompleteBeingUsedByCompositor(surface); });

  if (!surface) {
    return false;
  }

  auto texture = surface->GetTextureSource();

  if (!texture || !IsValid()) {
    VALIDATION_LOG << "Invalid texture to present.";
    return false;
  }

  android::SurfaceTransaction transaction;
  if (!transaction.SetContents(surface_control_.get(),
                               texture->GetHardwareBuffer().get())) {
    VALIDATION_LOG << "Could not set surface contents.";
    return false;
  }

  if (!transaction.Apply([surface, weak = weak_from_this()]() {
        auto thiz = weak.lock();
        if (!thiz) {
          return;
        }
        thiz->OnSurfaceDidCompleteBeingUsedByCompositor(surface);
      })) {
    VALIDATION_LOG << "Could not apply surface transaction.";
    return false;
  }

  // The compositor owns our surface now and will recycle the texture in the
  // completion callback.
  recycle_on_fail.Release();
  return true;
}

void AHBSwapchainVK::OnSurfaceDidCompleteBeingUsedByCompositor(
    const std::shared_ptr<AHBSurfaceVK>& surface) {
  {
    Lock lock(mutex_);
    PushRecyclable(surface);
  }
  drawable_count_sema_.Signal();
}

void AHBSwapchainVK::PushRecyclable(
    const std::shared_ptr<AHBSurfaceVK>& surface) {
  // recyclable_.push_back(Recyclable{std::move(surface)});
  // FML_LOG(IMPORTANT) << "There are " << recyclable_.size() << "
  // recyclables.";
}

std::shared_ptr<AHBSurfaceVK> AHBSwapchainVK::PopRecyclable() {
  return nullptr;
  // if (recyclable_.empty()) {
  //   return nullptr;
  // }
  // auto surface = recyclable_.back().surface;
  // recyclable_.pop_back();
  // return surface;
}

}  // namespace impeller
