// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_surface_delegate.h"

namespace flutter {

GPUSurfaceDelegate::~GPUSurfaceDelegate() = default;

SkPixelGeometry GPUSurfaceDelegate::GetPixelGeometry() const {
  return kUnknown_SkPixelGeometry;
}

}  // namespace flutter
