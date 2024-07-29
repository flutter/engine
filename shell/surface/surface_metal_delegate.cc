// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/surface/surface_metal_delegate.h"

namespace flutter {

SurfaceMetalDelegate::SurfaceMetalDelegate(
    MTLRenderTargetType render_target_type)
    : render_target_type_(render_target_type) {}

SurfaceMetalDelegate::~SurfaceMetalDelegate() = default;

MTLRenderTargetType SurfaceMetalDelegate::GetRenderTargetType() {
  return render_target_type_;
}

bool SurfaceMetalDelegate::AllowsDrawingWhenGpuDisabled() const {
  return true;
}

}  // namespace flutter
