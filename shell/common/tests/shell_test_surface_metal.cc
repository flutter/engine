// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/tests/shell_test_surface_metal.h"

#include "flutter/shell/common/tests/shell_test_surface_metal_impl.h"

namespace flutter {
namespace testing {

std::unique_ptr<ShellTestSurfaceMetal> ShellTestSurfaceMetal::Create() {
  std::unique_ptr<ShellTestSurface> impl;
#if SHELL_ENABLE_METAL
  impl = std::make_unique<ShellTestSurfaceMetalImpl>();
#endif  // SHELL_ENABLE_METAL
  // Cannot use make_unique because of the private constructor.
  return std::unique_ptr<ShellTestSurfaceMetal>{
      new ShellTestSurfaceMetal(std::move(impl))};
}

ShellTestSurfaceMetal::ShellTestSurfaceMetal(
    std::unique_ptr<ShellTestSurface> impl)
    : impl_(std::move(impl)) {}

ShellTestSurfaceMetal::~ShellTestSurfaceMetal() = default;

// |ShellTestSurface|
std::unique_ptr<Surface> ShellTestSurfaceMetal::CreateRenderingSurface() {
  return impl_ ? impl_->CreateRenderingSurface() : nullptr;
}

}  // namespace testing
}  // namespace flutter
