// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_GLFW_PLATFORM_VIEW_GLFW_H_
#define SHELL_PLATFORM_GLFW_PLATFORM_VIEW_GLFW_H_

#include <string>
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/gpu/gpu_surface_gl.h"
#include "lib/ftl/memory/weak_ptr.h"

struct GLFWwindow;

namespace shell {

class PlatformViewGLFW : public PlatformView, public GPUSurfaceGLDelegate {
 public:
  PlatformViewGLFW();

  ~PlatformViewGLFW() override;

  void InitMessageLoop();

  void Attach() override;

  void Run(const std::string& main, const std::string& packages);

  void RunFromSource(const std::string& assets_directory,
                     const std::string& main,
                     const std::string& packages) override;

  void RunMessageLoop();

  void Cleanup();

  bool SurfaceSupportsSRGB() const override;

  bool ResourceContextMakeCurrent() override;

  bool GLContextMakeCurrent() override;

  bool GLContextClearCurrent() override;

  bool GLContextPresent() override;

  intptr_t GLContextFBO() const override;

 private:
  GLFWwindow* glfw_window_;

  FTL_DISALLOW_COPY_AND_ASSIGN(PlatformViewGLFW);
};

}  // namespace shell

#endif  // SHELL_PLATFORM_GLFW_PLATFORM_VIEW_GLFW_H_
