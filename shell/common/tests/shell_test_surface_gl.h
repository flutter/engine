// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_GL_H_
#define FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_GL_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/common/tests/shell_test_surface.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#include "flutter/testing/test_gl_surface.h"

namespace flutter {
namespace testing {

class ShellTestSurfaceGL : public ShellTestSurface,
                           public GPUSurfaceGLDelegate {
 public:
  ShellTestSurfaceGL();

  virtual ~ShellTestSurfaceGL();

 private:
  TestGLSurface gl_surface_;

  // |ShellTestSurface|
  std::unique_ptr<Surface> CreateRenderingSurface() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextMakeCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextClearCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextPresent() override;

  // |GPUSurfaceGLDelegate|
  intptr_t GLContextFBO() const override;

  // |GPUSurfaceGLDelegate|
  GLProcResolver GetGLProcResolver() const override;

  // |GPUSurfaceGLDelegate|
  ExternalViewEmbedder* GetExternalViewEmbedder() override;

  FML_DISALLOW_COPY_AND_ASSIGN(ShellTestSurfaceGL);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_SURFACE_GL_H_
