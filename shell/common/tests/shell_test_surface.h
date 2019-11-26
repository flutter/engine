// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_H_
#define FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/common/surface.h"

namespace flutter {
namespace testing {

class ShellTestSurface {
 public:
  enum class ClientRenderingAPI {
    kClientRenderingAPIOpenGL,
    kClientRenderingAPIMetal,
  };

  static std::unique_ptr<ShellTestSurface> CreateTestSurface(
      ClientRenderingAPI api);

  ShellTestSurface();

  virtual ~ShellTestSurface();

  virtual std::unique_ptr<Surface> CreateRenderingSurface() = 0;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(ShellTestSurface);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_H_
