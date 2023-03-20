// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_studio_vulkan.h"

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"

#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkSize.h"
#include "vulkan/vulkan_core.h"

namespace flutter {

GPUStudioVulkan::GPUStudioVulkan(const sk_sp<GrDirectContext>& skia_context)
    : skia_context_(skia_context) {}

GPUStudioVulkan::~GPUStudioVulkan() = default;

bool GPUStudioVulkan::IsValid() {
  return skia_context_ != nullptr;
}

GrDirectContext* GPUStudioVulkan::GetContext() {
  return skia_context_.get();
}

SkColorType GPUStudioVulkan::ColorTypeFromFormat(const VkFormat format) {
  switch (format) {
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SRGB:
      return SkColorType::kRGBA_8888_SkColorType;
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SRGB:
      return SkColorType::kBGRA_8888_SkColorType;
    default:
      return SkColorType::kUnknown_SkColorType;
  }
}

}  // namespace flutter
