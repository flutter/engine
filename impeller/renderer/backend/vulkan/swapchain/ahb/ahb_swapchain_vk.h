// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_AHB_AHB_SWAPCHAIN_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_AHB_AHB_SWAPCHAIN_VK_H_

#include <deque>
#include <memory>

#include "flutter/fml/synchronization/semaphore.h"
#include "impeller/base/thread.h"
#include "impeller/base/timing.h"
#include "impeller/renderer/backend/vulkan/android/ahb_texture_source_vk.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/swapchain_vk.h"
#include "impeller/renderer/backend/vulkan/texture_vk.h"
#include "impeller/toolkit/android/hardware_buffer.h"
#include "impeller/toolkit/android/native_window.h"
#include "impeller/toolkit/android/surface_control.h"

namespace impeller {

class AHBSurfaceVK;

class AHBSwapchainVK final
    : public SwapchainVK,
      public std::enable_shared_from_this<AHBSwapchainVK> {
 public:
  static std::shared_ptr<AHBSwapchainVK> Create(
      const std::shared_ptr<Context>& context,
      ANativeWindow* window,
      size_t max_drawable_count = 3u);

  // |SwapchainVK|
  ~AHBSwapchainVK() override;

  AHBSwapchainVK(const AHBSwapchainVK&) = delete;

  AHBSwapchainVK& operator=(const AHBSwapchainVK&) = delete;

  // |SwapchainVK|
  bool IsValid() const override;

  // |SwapchainVK|
  vk::Format GetSurfaceFormat() const override;

  // |SwapchainVK|
  std::shared_ptr<Surface> AcquireNextDrawable() override;

  // |SwapchainVK|
  void UpdateSurfaceSize(const ISize& size) override;

  bool PresentSurface(const std::shared_ptr<AHBSurfaceVK>& surface);

 private:
  struct Recyclable {
    TimePoint last_use;
    std::shared_ptr<AHBSurfaceVK> surface;

    explicit Recyclable(std::shared_ptr<AHBSurfaceVK> p_surface)
        : last_use(Clock::now()), surface(std::move(p_surface)) {}
  };

  std::weak_ptr<Context> context_;
  std::shared_ptr<android::NativeWindow> native_window_;
  std::shared_ptr<android::SurfaceControl> surface_control_;
  fml::Semaphore drawable_count_sema_;
  mutable Mutex mutex_;
  android::HardwareBufferDescriptor desc_ IPLR_GUARDED_BY(mutex_);
  std::deque<Recyclable> recyclable_ IPLR_GUARDED_BY(mutex_);
  bool is_valid_ = false;

  AHBSwapchainVK(const std::shared_ptr<Context>& context,
                 ANativeWindow* window,
                 size_t max_drawable_count);

  void OnSurfaceDidCompleteBeingUsedByCompositor(
      const std::shared_ptr<AHBSurfaceVK>& surface);

  std::shared_ptr<AHBSurfaceVK> CreateNewSurface() IPLR_REQUIRES(mutex_);

  void PushRecyclable(const std::shared_ptr<AHBSurfaceVK>& surface)
      IPLR_REQUIRES(mutex_);

  std::shared_ptr<AHBSurfaceVK> PopRecyclable() IPLR_REQUIRES(mutex_);
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_AHB_AHB_SWAPCHAIN_VK_H_
