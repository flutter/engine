// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"

#include "flutter/common/graphics/persistent_cache.h"

namespace flutter {

GPUSurfaceMetalDelegate::GPUSurfaceMetalDelegate(
    MTLRenderTargetType render_target_type)
    : render_target_type_(render_target_type) {}

GPUSurfaceMetalDelegate::~GPUSurfaceMetalDelegate() = default;

MTLRenderTargetType GPUSurfaceMetalDelegate::GetRenderTargetType() {
  return render_target_type_;
}

bool GPUSurfaceMetalDelegate::AllowsDrawingWhenGpuDisabled() const {
  return true;
}

GPUSurfaceMetalDelegate::SkSLPrecompiler::SkSLPrecompiler() {}

void GPUSurfaceMetalDelegate::SkSLPrecompiler::PrecompileKnownSkSLsIfNecessary(
    GrDirectContext* current_context) {
  if (current_context == precompiled_sksl_context_) {
    // Known SkSLs have already been prepared in this context.
    return;
  }
  precompiled_sksl_context_ = current_context;
  flutter::PersistentCache::GetCacheForProcess()->PrecompileKnownSkSLs(
      precompiled_sksl_context_);
}

}  // namespace flutter
