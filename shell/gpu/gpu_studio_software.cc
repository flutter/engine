// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_studio_software.h"

#include <memory>

#include "flutter/fml/logging.h"

namespace flutter {

GPUStudioSoftware::GPUStudioSoftware(GPUSurfaceSoftwareDelegate* delegate)
    : delegate_(delegate) {}

GPUStudioSoftware::~GPUStudioSoftware() = default;

// |Studio|
bool GPUStudioSoftware::IsValid() {
  return delegate_ != nullptr;
}

// |Studio|
GrDirectContext* GPUStudioSoftware::GetContext() {
  // There is no GrContext associated with a software surface.
  return nullptr;
}

}  // namespace flutter
