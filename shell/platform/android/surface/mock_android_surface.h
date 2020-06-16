// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_MOCK_ANDROID_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_MOCK_ANDROID_SURFACE_H_

#include "flutter/shell/gpu/gpu_surface_gl.h"
#include "flutter/shell/platform/android/surface/android_surface.h"

namespace flutter {

//------------------------------------------------------------------------------
/// Mock for |AndroidSurface|. This implementation can be used in unit
/// tests without requiring the Android toolchain.
///
class MockAndroidSurface final : public GPUSurfaceGLDelegate,
                                 public AndroidSurface {
 public:
  MockAndroidSurface(int id);

  ~MockAndroidSurface() override;

  // |AndroidSurface|
  bool IsValid() const override;

  // |AndroidSurface|
  void TeardownOnScreenContext() override;

  // |AndroidSurface|
  std::unique_ptr<Surface> CreateGPUSurface(GrContext* gr_context) override;

  // |AndroidSurface|
  bool OnScreenSurfaceResize(const SkISize& size) override;

  // |AndroidSurface|
  bool ResourceContextMakeCurrent() override;

  // |AndroidSurface|
  bool ResourceContextClearCurrent() override;

  // |AndroidSurface|
  bool SetNativeWindow(fml::RefPtr<AndroidNativeWindow> window) override;

  // |GPUSurfaceGLDelegate|
  std::unique_ptr<GLContextResult> GLContextMakeCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextClearCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextPresent() override;

  // |GPUSurfaceGLDelegate|
  intptr_t GLContextFBO() const override;

  // |GPUSurfaceGLDelegate|
  ExternalViewEmbedder* GetExternalViewEmbedder() override;

  // Used in unit tests.
  int GetId() const;

 private:
  int id_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_MOCK_ANDROID_SURFACE_H_
