// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/tests/shell_test_surface.h"

#include "flutter/shell/common/tests/shell_test_surface_gl.h"
#include "flutter/shell/common/tests/shell_test_surface_metal.h"
#include "flutter/testing/test_metal_surface.h"

namespace flutter {
namespace testing {

std::unique_ptr<ShellTestSurface> ShellTestSurface::CreateTestSurface(
    ClientRenderingAPI api) {
  switch (api) {
    case ClientRenderingAPI::kClientRenderingAPIOpenGL:
      // OpenGL is always available on all platform because the test harnesses
      // emulate the same via SwiftShader.
      return std::make_unique<ShellTestSurfaceGL>();
    case ClientRenderingAPI::kClientRenderingAPIMetal:
      FML_CHECK(TestMetalSurface::PlatformSupportsMetal())
          << "Attempted to use or test a Metal feature on a platform where "
             "Metal is not available and cannot be emulated. If not testing "
             "anything Metal specific, pick a different client rendering API "
             "(OpenGL recommended as it is emulated via SwiftShader). If "
             "testing a Metal subsystem, skip the test conditionally on "
             "non-Darwin platforms via GTEST_SKIP.";
      return ShellTestSurfaceMetal::Create();
  }
  FML_CHECK(false) << "Unhandled client rendering API.";
  return nullptr;
}

ShellTestSurface::ShellTestSurface() = default;

ShellTestSurface::~ShellTestSurface() = default;

}  // namespace testing
}  // namespace flutter
