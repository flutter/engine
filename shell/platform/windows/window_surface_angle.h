// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_SURFACE_ANGLE_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_SURFACE_ANGLE_H_

#include "flutter/shell/platform/windows/angle_surface_manager.h"
#include "flutter/shell/platform/windows/window_surface.h"

namespace flutter {

// Represents a rendering surface for a platform window.
class WindowSurfaceAngle : public WindowSurface {
public:
  WindowSurfaceAngle(AngleSurfaceManager* surface_manager);
  virtual ~WindowSurfaceAngle();

  // |WindowSurface|
  void Init(PlatformWindow window, unsigned int width, unsigned int height) override;

  // |WindowSurface|
  void Destroy() override;

  // |WindowSurface|
  void GetSurfaceDimensions(unsigned int &width, unsigned int &height) override;

  // |WindowSurface|
  void OnResize(unsigned int width, unsigned int height) override;

  // |WindowSurface|
  void InitRendererConfig(FlutterRendererConfig& config) override;

  // |WindowSurface|
  void CleanUpRendererConfig(FlutterRendererConfig& config) override;

private:
  PlatformWindow window_;
  // TODO: This should just own the AngleSurfaceManager.
  AngleSurfaceManager* surface_manager_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_SURFACE_ANGLE_H_

