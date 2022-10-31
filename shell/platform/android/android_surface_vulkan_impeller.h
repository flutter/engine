// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once


#include "flutter/fml/macros.h"
#include "flutter/impeller/renderer/context.h"
#include "flutter/shell/gpu/gpu_surface_vulkan_delegate.h"
#include "flutter/shell/platform/android/surface/android_native_window.h"
#include "flutter/shell/platform/android/surface/android_surface.h"

namespace flutter {

class AndroidSurfaceVulkanImpeller : public AndroidSurface,
                             public GPUSurfaceVulkanDelegate {
 public:
  AndroidSurfaceVulkanImpeller(const std::shared_ptr<AndroidContext>& android_context,
                       std::shared_ptr<PlatformViewAndroidJNI> jni_facade);

  ~AndroidSurfaceVulkanImpeller() override;

  // |AndroidSurface|
  bool IsValid() const override;

  // |AndroidSurface|
  std::unique_ptr<Surface> CreateGPUSurface(
      GrDirectContext* gr_context) override;

  // |AndroidSurface|
  void TeardownOnScreenContext() override;

  // |AndroidSurface|
  bool OnScreenSurfaceResize(const SkISize& size) override;

  // |AndroidSurface|
  bool ResourceContextMakeCurrent() override;

  // |AndroidSurface|
  bool ResourceContextClearCurrent() override;

  // |AndroidSurface|
  bool SetNativeWindow(fml::RefPtr<AndroidNativeWindow> window) override;

  // |GPUSurfaceVulkanDelegate|
  const vulkan::VulkanProcTable& vk() override;

 private:
  fml::RefPtr<vulkan::VulkanProcTable> proc_table_;
  fml::RefPtr<AndroidNativeWindow> native_window_;

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidSurfaceVulkanImpeller);
};

}  // namespace flutter
