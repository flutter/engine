// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_METAL_IMPL_H_
#define FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_METAL_IMPL_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/common/tests/shell_test_surface_metal.h"
#include "flutter/shell/gpu/gpu_surface_delegate.h"

namespace flutter {
namespace testing {

class ShellTestSurfaceMetalImpl : public ShellTestSurface,
                                  public GPUSurfaceDelegate {
 public:
  ShellTestSurfaceMetalImpl();

  ~ShellTestSurfaceMetalImpl();

 private:
  // |ShellTestSurface|
  std::unique_ptr<Surface> CreateRenderingSurface() override;

  // |GPUSurfaceDelegate|
  ExternalViewEmbedder* GetExternalViewEmbedder() override;

  FML_DISALLOW_COPY_AND_ASSIGN(ShellTestSurfaceMetalImpl);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_METAL_IMPL_H_
