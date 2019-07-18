// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_ANGLE_SURFACE_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_ANGLE_SURFACE_MANAGER_H_

// OpenGL ES includes
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// EGL includes
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

namespace flutter {

// An implementation for inializing ANGLE correctly and using it to create and
// destroy surfaces
class AngleSurfaceManager {
 public:
  AngleSurfaceManager();
  ~AngleSurfaceManager();

  EGLSurface CreateSurface(HWND window);
  void GetSurfaceDimensions(const EGLSurface surface,
                            EGLint* width,
                            EGLint* height);
  void DestroySurface(const EGLSurface surface);
  bool MakeCurrent(const EGLSurface surface);
  EGLBoolean SwapBuffers(const EGLSurface surface);

 private:
  bool Initialize();
  void CleanUp();

 private:
  EGLDisplay egl_display_;
  EGLContext egl_context_;
  EGLConfig egl_config_;
  bool initialize_succeeded_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_ANGLE_SURFACE_MANAGER_H_
