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

#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkStudio.h"

namespace flutter {

//------------------------------------------------------------------------------
/// @brief  A GPU surface backed by VkImages provided by a
///         GPUStudioVulkanDelegate.
///
class GPUStudioVulkan : public Studio {
 public:
  //------------------------------------------------------------------------------
  /// @brief      Create a GPUStudioVulkan while letting it reuse an existing
  ///             GrDirectContext.
  ///
  GPUStudioVulkan(GPUStudioVulkanDelegate* delegate,
                   const sk_sp<GrDirectContext>& context,
                   bool render_to_surface);

  ~GPUStudioVulkan() override;

  // |Studio|
  bool IsValid() override;

  // |Studio|
  GrDirectContext* GetContext() override;

  static SkColorType ColorTypeFromFormat(const VkFormat format);

 private:
  GPUStudioVulkanDelegate* delegate_;
  sk_sp<GrDirectContext> skia_context_;
  bool render_to_surface_;

  fml::WeakPtrFactory<GPUSurfaceVulkan> weak_factory_;

  FML_DISALLOW_COPY_AND_ASSIGN(GPUStudioVulkan);
};

}  // namespace flutter

#endif  // SHELL_GPU_GPU_SURFACE_VULKAN_H_
