// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/tests/shell_test_surface_metal_impl.h"

#include <Foundation/Foundation.h>
#include <QuartzCore/CAMetalLayer.h>

#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/shell/gpu/gpu_surface_metal.h"

namespace flutter {
namespace testing {

ShellTestSurfaceMetalImpl::ShellTestSurfaceMetalImpl() = default;

ShellTestSurfaceMetalImpl::~ShellTestSurfaceMetalImpl() = default;

std::unique_ptr<Surface> ShellTestSurfaceMetalImpl::CreateRenderingSurface() {
  fml::scoped_nsobject<CAMetalLayer> layer([[CAMetalLayer alloc] init]);
  return std::make_unique<GPUSurfaceMetal>(this, layer);
}

ExternalViewEmbedder* ShellTestSurfaceMetalImpl::GetExternalViewEmbedder() {
  return nullptr;
}

}  // namespace testing
}  // namespace flutter
