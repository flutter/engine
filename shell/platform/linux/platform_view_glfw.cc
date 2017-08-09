// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/platform_view_glfw.h"

#include <GLFW/glfw3.h>
#include <iostream>

#include "flutter/common/threads.h"
#include "flutter/fml/message_loop.h"
#include "flutter/shell/gpu/gpu_rasterizer.h"

namespace shell {

inline PlatformViewGLFW* ToPlatformView(GLFWwindow* window) {
  return static_cast<PlatformViewGLFW*>(glfwGetWindowUserPointer(window));
}

PlatformViewGLFW::PlatformViewGLFW()
    : PlatformView(std::make_unique<GPURasterizer>(nullptr)),
      glfw_window_(nullptr) {
}

PlatformViewGLFW::~PlatformViewGLFW() {
  if (glfw_window_ != nullptr) {
    glfwSetWindowUserPointer(glfw_window_, nullptr);
    glfwDestroyWindow(glfw_window_);
    glfw_window_ = nullptr;
  }

  glfwTerminate();
}

void PlatformViewGLFW::Attach() {
  if (!glfwInit()) {
    return;
  }

  glfw_window_ = glfwCreateWindow(1920, 1080, "Flutter", NULL, NULL);
  if (glfw_window_ == nullptr) {
    return;
  }

  CreateEngine();

    blink::ViewportMetrics metrics;
  metrics.physical_width = 1920;
  metrics.physical_height = 1080;
  engine().SetViewportMetrics(metrics);


  glfwSetWindowUserPointer(glfw_window_, this);

  PostAddToShellTask();
}

bool PlatformViewGLFW::SurfaceSupportsSRGB() const {
    return false;
}

intptr_t PlatformViewGLFW::GLContextFBO() const {
  // The default window bound FBO.
  return 0;
}

bool PlatformViewGLFW::GLContextMakeCurrent() {
  glfwMakeContextCurrent(glfw_window_);
  return true;
}

bool PlatformViewGLFW::GLContextClearCurrent() {
  glfwMakeContextCurrent(nullptr);
  return true;
}

bool PlatformViewGLFW::ResourceContextMakeCurrent() {
  // Resource loading contexts are not supported on this platform.
  return false;
}

bool PlatformViewGLFW::GLContextPresent() {
  glfwSwapBuffers(glfw_window_);
  return true;
}

void PlatformViewGLFW::Run(const std::string& main, const std::string& packages) {
  RunFromSource(std::string(), main, packages);
}

void PlatformViewGLFW::RunFromSource(const std::string& assets_directory,
                                     const std::string& main,
                                     const std::string& packages) {
  blink::Threads::UI()->PostTask(
      [ engine = engine().GetWeakPtr(), main, packages ] {
        if (engine) {
          if (true) { // IsDartFile(target)) {
            engine->RunBundleAndSource(std::string(), main, packages);
          } else {
            engine->RunBundle(main);
          }
        }
      });
}

void PlatformViewGLFW::InitMessageLoop() {
  // This is a platform thread (i.e not one created by fml::Thread), so perform
  // one time initialization.
  fml::MessageLoop::EnsureInitializedForCurrentThread();
}

void PlatformViewGLFW::RunMessageLoop() {
  fml::MessageLoop::GetCurrent().Run();
}

void PlatformViewGLFW::Cleanup() {
  NotifyDestroyed();
}

}  // namespace shell
