// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_METAL_H_
#define FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_METAL_H_

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/shell/common/tests/shell_test_surface.h"

namespace flutter {
namespace testing {

class ShellTestSurfaceMetal : public ShellTestSurface {
 public:
  static std::unique_ptr<ShellTestSurfaceMetal> Create();

  virtual ~ShellTestSurfaceMetal();

 private:
  std::unique_ptr<ShellTestSurface> impl_;

  ShellTestSurfaceMetal(std::unique_ptr<ShellTestSurface> impl);

  // |ShellTestSurface|
  std::unique_ptr<Surface> CreateRenderingSurface() override;

  FML_DISALLOW_COPY_AND_ASSIGN(ShellTestSurfaceMetal);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_METAL_H_
