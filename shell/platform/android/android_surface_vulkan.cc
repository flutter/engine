// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_surface_vulkan.h"
#include "flutter/shell/gpu/gpu_surface_vulkan.h"
#include "flutter/vulkan/vulkan_native_surface_android.h"
#include "lib/ftl/logging.h"

namespace shell {

AndroidSurfaceVulkan::AndroidSurfaceVulkan(
    ftl::RefPtr<AndroidNativeWindow> native_window)
    : valid_(false), native_window_(native_window) {
  if (!native_window_ || !native_window_->IsValid()) {
    return;
  }

  // TODO(chinmaygarde): More logic to check if Vulkan is actually available on
  // this platform goes here.

  valid_ = true;
}

AndroidSurfaceVulkan::~AndroidSurfaceVulkan() = default;

bool AndroidSurfaceVulkan::IsValid() const {
  return valid_;
}

std::unique_ptr<Surface> AndroidSurfaceVulkan::CreateGPUSurface() {
  if (!IsValid()) {
    return nullptr;
  }

  auto vulkan_surface_android =
      std::make_unique<vulkan::VulkanNativeSurfaceAndroid>(
          native_window_->handle());

  if (!vulkan_surface_android->IsValid()) {
    return nullptr;
  }

  auto gpu_surface =
      std::make_unique<GPUSurfaceVulkan>(std::move(vulkan_surface_android));

  if (!gpu_surface->IsValid()) {
    return nullptr;
  }

  if (!gpu_surface->Setup()) {
    return nullptr;
  }

  return gpu_surface;
}

SkISize AndroidSurfaceVulkan::OnScreenSurfaceSize() const {
  return IsValid() ? native_window_->GetSize() : SkISize::Make(0, 0);
}

bool AndroidSurfaceVulkan::OnScreenSurfaceResize(const SkISize& size) const {
  return true;
}

bool AndroidSurfaceVulkan::ResourceContextMakeCurrent() {
  return false;
}

}  // namespace shell
