// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/platform_view_glfw.h"

#include <GLFW/glfw3.h>
#include <iostream>

#include "flutter/common/threads.h"
#include "flutter/shell/gpu/gpu_rasterizer.h"

namespace shell {

inline PlatformViewGLFW* ToPlatformView(GLFWwindow* window) {
  return static_cast<PlatformViewGLFW*>(glfwGetWindowUserPointer(window));
}

PlatformViewGLFW::PlatformViewGLFW()
    : PlatformView(std::make_unique<GPURasterizer>(nullptr)),
      valid_(false),
      glfw_window_(nullptr),
      buttons_(0) {
}

PlatformViewGLFW::~PlatformViewGLFW() {
  if (glfw_window_ != nullptr) {
    glfwSetWindowUserPointer(glfw_window_, nullptr);
    glfwDestroyWindow(glfw_window_);
    glfw_window_ = nullptr;
  }

  glfwTerminate();
}

bool PlatformViewGLFW::IsValid() const {
  return valid_;
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

  glfwSetWindowSizeCallback(
      glfw_window_, [](GLFWwindow* window, int width, int height) {
        ToPlatformView(window)->OnWindowSizeChanged(width, height);
      });

  glfwSetMouseButtonCallback(
      glfw_window_, [](GLFWwindow* window, int button, int action, int mods) {
        ToPlatformView(window)->OnMouseButtonChanged(button, action, mods);
      });

  glfwSetKeyCallback(glfw_window_, [](GLFWwindow* window, int key, int scancode,
                                      int action, int mods) {
    ToPlatformView(window)->OnKeyEvent(key, scancode, action, mods);
  });

  valid_ = true;

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
  std::cout << "swapping buffers\n";
  glfwSwapBuffers(glfw_window_);
  return true;
}

void PlatformViewGLFW::RunFromSource(const std::string& assets_directory,
                                     const std::string& main,
                                     const std::string& packages) {}

void PlatformViewGLFW::OnWindowSizeChanged(int width, int height) {
  std::cout << "window size changed\n";
  blink::ViewportMetrics metrics;
  metrics.physical_width = width;
  metrics.physical_height = height;

  blink::Threads::UI()->PostTask([ engine = engine().GetWeakPtr(), metrics ] {
    std::cout << engine.get();
    if (engine.get())
      engine->SetViewportMetrics(metrics);
  });
}

void PlatformViewGLFW::OnMouseButtonChanged(int button, int action, int mods) {
  blink::PointerData::Change change = blink::PointerData::Change::kCancel;
  if (action == GLFW_PRESS) {
    if (!buttons_) {
      change = blink::PointerData::Change::kDown;
      glfwSetCursorPosCallback(
          glfw_window_, [](GLFWwindow* window, double x, double y) {
            ToPlatformView(window)->OnCursorPosChanged(x, y);
          });
    } else {
      change = blink::PointerData::Change::kMove;
    }
    // GLFW's button order matches what we want:
    // https://github.com/flutter/engine/blob/master/sky/specs/pointer.md
    // http://www.glfw.org/docs/3.2/group__buttons.html
    buttons_ |= 1 << button;
  } else if (action == GLFW_RELEASE) {
    buttons_ &= ~(1 << button);
    if (!buttons_) {
      change = blink::PointerData::Change::kUp;
      glfwSetCursorPosCallback(glfw_window_, nullptr);
    } else {
      change = blink::PointerData::Change::kMove;
    }
  } else {
    FTL_DLOG(INFO) << "Unknown mouse action: " << action;
    return;
  }

  double x = 0.0;
  double y = 0.0;
  glfwGetCursorPos(glfw_window_, &x, &y);

  blink::PointerData pointer_data;
  pointer_data.Clear();
  pointer_data.time_stamp =
      ftl::TimePoint::Now().ToEpochDelta().ToMicroseconds();
  pointer_data.change = change;
  pointer_data.kind = blink::PointerData::DeviceKind::kMouse;
  pointer_data.physical_x = x;
  pointer_data.physical_y = y;
  pointer_data.buttons = buttons_;
  pointer_data.pressure = 1.0;
  pointer_data.pressure_max = 1.0;

  blink::Threads::UI()->PostTask(
      [ engine = engine().GetWeakPtr(), pointer_data ] {
        if (engine.get()) {
          blink::PointerDataPacket packet(1);
          packet.SetPointerData(0, pointer_data);
          engine->DispatchPointerDataPacket(packet);
        }
      });
}

void PlatformViewGLFW::OnCursorPosChanged(double x, double y) {
  blink::PointerData pointer_data;
  pointer_data.Clear();
  pointer_data.time_stamp =
      ftl::TimePoint::Now().ToEpochDelta().ToMicroseconds();
  pointer_data.change = blink::PointerData::Change::kMove;
  pointer_data.kind = blink::PointerData::DeviceKind::kMouse;
  pointer_data.physical_x = x;
  pointer_data.physical_y = y;
  pointer_data.buttons = buttons_;
  pointer_data.pressure = 1.0;
  pointer_data.pressure_max = 1.0;

  blink::Threads::UI()->PostTask(
      [ engine = engine().GetWeakPtr(), pointer_data ] {
        if (engine.get()) {
          blink::PointerDataPacket packet(1);
          packet.SetPointerData(0, pointer_data);
          engine->DispatchPointerDataPacket(packet);
        }
      });
}

void PlatformViewGLFW::OnKeyEvent(int key, int scancode, int action, int mods) {
}

}  // namespace shell
