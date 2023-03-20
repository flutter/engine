// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_GPU_GPU_STUDIO_VULKAN_H_
#define SHELL_GPU_GPU_STUDIO_VULKAN_H_

#include <memory>

#include "flutter/flow/studio.h"
#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/gpu/gpu_surface_vulkan_delegate.h"
#include "flutter/vulkan/vulkan_backbuffer.h"
#include "flutter/vulkan/vulkan_native_surface.h"
#include "flutter/vulkan/vulkan_window.h"

#include "third_party/skia/include/core/SkRefCnt.h"

namespace flutter {

//------------------------------------------------------------------------------
/// @brief  A GPU surface backed by VkImages provided by a
///         GPUSurfaceVulkanDelegate.
///
class GPUStudioVulkan : public Studio {
 public:
  //------------------------------------------------------------------------------
  /// @brief      Create a GPUStudioVulkan while letting it reuse an existing
  ///             GrDirectContext.
  ///
  GPUStudioVulkan(const sk_sp<GrDirectContext>& context);

  ~GPUStudioVulkan() override;

  // |Studio|
  bool IsValid() override;

  // |Studio|
  GrDirectContext* GetContext() override;

  static SkColorType ColorTypeFromFormat(const VkFormat format);

 private:
  sk_sp<GrDirectContext> skia_context_;

  FML_DISALLOW_COPY_AND_ASSIGN(GPUStudioVulkan);
};

}  // namespace flutter

#endif  // SHELL_GPU_GPU_STUDIO_VULKAN_H_
