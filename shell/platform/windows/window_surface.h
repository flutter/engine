// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_SURFACE_H_

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/window_binding_handler.h"

namespace flutter {

class PlatformViewRegistrar;

class WindowSurfaceHook {
public:
  virtual ~WindowSurfaceHook() = default;

  // Called before retrieving the FBO (OpenGL) or Swapchain Image (D3D12).
  virtual void OnAcquireFrame(unsigned int width, unsigned int height) = 0;

  // Called right before calling SwapBuffers (OpenGL) or Present (D3D12).
  // Return true to continue present, return false to cancel.
  virtual bool OnPresentFramePre() = 0;

  // Called right after calling SwapBuffers (OpenGL) or Present (D3D12).
  // Not called if canceled by OnPresentFramePre.
  virtual void OnPresentFramePost() = 0;
};

// Represents a rendering surface for a platform window.
class WindowSurface {
public:
  virtual ~WindowSurface() = default;

  // Initializes the surface with the given platform window.
  virtual void Init(PlatformWindow window, unsigned int width, unsigned int height) = 0;

  // Destroys the surface previously initialized by Init.
  virtual void Destroy() = 0;

  virtual void GetSurfaceDimensions(unsigned int &width, unsigned int &height) = 0;

  // Resizes the surface to match the new size.
  virtual void OnResize(unsigned int width, unsigned int height) = 0;

  // Fills out the given renderer config describing how to render into the surface.
  virtual void InitRendererConfig(FlutterRendererConfig& config) = 0;

  // Callback to dispose any memory allocated for the renderer config.
  virtual void CleanUpRendererConfig(FlutterRendererConfig& config) = 0;

  // TODO: Move to a separate WindowCompositor class.
  // Fills out the given compositor config.
  // Return true if the config was filled out successfully.
  virtual bool InitCompositorConfig(FlutterCompositor& config) { return false; }

  // Callback to dispose any memory allocated for the compositor config.
  virtual void CleanUpCompositorConfig(FlutterCompositor& config) {}

  // Returns the IDCompositionDevice used to composite the surface.
  virtual void* GetCompositionDevice() {
    return nullptr;
  }

  void SetWindowSurfaceHook(WindowSurfaceHook* hook) {
    hook_ = hook;
  }

  void SetPlatformViewRegistrar(PlatformViewRegistrar* platform_views) {
    platform_views_ = platform_views;
  }

protected:

  WindowSurfaceHook* GetWindowSurfaceHook() { return hook_; }

  // TODO: This should move to the compositor, once it is separated from the surface.
  PlatformViewRegistrar* platform_views_ = nullptr;

private:
  WindowSurfaceHook* hook_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_SURFACE_H_

