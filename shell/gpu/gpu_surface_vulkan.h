// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_GPU_GPU_SURFACE_VULKAN_H_
#define SHELL_GPU_GPU_SURFACE_VULKAN_H_

#include <memory>

#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/gpu/gpu_surface_vulkan_delegate.h"
#include "flutter/vulkan/vulkan_backbuffer.h"
#include "flutter/vulkan/vulkan_native_surface.h"
#include "flutter/vulkan/vulkan_window.h"
#include "include/core/SkRefCnt.h"

namespace flutter {


//------------------------------------------------------------------------------
/// @brief  A GPU surface backed by VkImages provided by a
///         GPUSurfaceVulkanDelegate.
///
class GPUSurfaceVulkan : public Surface {
 public:
  //------------------------------------------------------------------------------
  /// @brief      Create a GPUSurfaceVulkan while letting it reuse an existing
  ///             GrDirectContext.
  ///
  GPUSurfaceVulkan(const sk_sp<GrDirectContext>& context,
                   GPUSurfaceVulkanDelegate* delegate);

  ~GPUSurfaceVulkan() override;

  // |Surface|
  bool IsValid() override;

  // |Surface|
  std::unique_ptr<SurfaceFrame> AcquireFrame(const SkISize& size) override;

  /// @brief  Called when a frame is done rendering. It blocks on the render
  ///         fence and then calls `GPUSurfaceVulkanDelegate::PresentImage` with
  ///         the populated image.
  bool Present();

  // |Surface|
  SkMatrix GetRootTransformation() const override;

  // |Surface|
  GrDirectContext* GetContext() override;

 private:
  sk_sp<GrDirectContext> skia_context_;
  GPUSurfaceVulkanDelegate* delegate_;

  std::vector<std::unique_ptr<VulkanBackbuffer>> backbuffers_;
  std::vector<sk_sp<SkSurface>> surfaces_;
  size_t current_backbuffer_index_;

  fml::WeakPtrFactory<GPUSurfaceVulkan> weak_factory_;
  FML_DISALLOW_COPY_AND_ASSIGN(GPUSurfaceVulkan);
};

}  // namespace flutter

#endif  // SHELL_GPU_GPU_SURFACE_VULKAN_H_
