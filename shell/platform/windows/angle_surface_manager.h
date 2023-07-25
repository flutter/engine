// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_ANGLE_SURFACE_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_ANGLE_SURFACE_MANAGER_H_

// OpenGL ES and EGL includes
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// Windows platform specific includes
#include <d3d11.h>
#include <windows.h>
#include <wrl/client.h>
#include <memory>
#include <unordered_set>

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/windows/window_binding_handler.h"

namespace flutter {

// A manager for initializing ANGLE correctly and using it to create and
// destroy surfaces
class AngleSurfaceManager {
 public:
  static std::unique_ptr<AngleSurfaceManager> Create(bool enable_impeller);

  virtual ~AngleSurfaceManager();

  // Returns true if the OpenGL version is greater than or equal to the input.
  bool GlVersion(int major, int minor);

  // Returns true if the OpenGL extension is available.
  bool HasExtension(std::string extension);

  // Creates an EGLSurface wrapper and backing DirectX 11 SwapChain
  // associated with window, in the appropriate format for display.
  // Target represents the visual entity to bind to.  Width and
  // height represent dimensions surface is created at.
  virtual bool CreateSurface(int64_t surface_id,
                             WindowsRenderTarget* render_target,
                             EGLint width,
                             EGLint height,
                             bool enable_vsync);

  // Resizes backing surface from current size to newly requested size
  // based on width and height for the specific case when width and height do
  // not match current surface dimensions.  Target represents the visual entity
  // to bind to.
  virtual void ResizeSurface(int64_t surface_id,
                             WindowsRenderTarget* render_target,
                             EGLint width,
                             EGLint height,
                             bool enable_vsync);

  // queries EGL for the dimensions of surface in physical
  // pixels returning width and height as out params.
  void GetSurfaceDimensions(int64_t surface_id, EGLint* width, EGLint* height);

  // Releases the pass-in EGLSurface wrapping and backing resources if not null.
  virtual void DestroySurface(int64_t surface_id);

  // Binds egl_context_ to the current rendering thread. Returns true on
  // success.
  bool MakeRenderContextCurrent();

  // Binds egl_context_ to the current rendering thread and to the draw and read
  // surfaces returning a boolean result reflecting success.
  bool MakeSurfaceCurrent(int64_t surface_id);

  // Clears current egl_context_
  bool ClearContext();

  // Binds egl_resource_context_ to the current rendering thread and to the draw
  // and read surfaces returning a boolean result reflecting success.
  bool MakeResourceCurrent();

  // Swaps the front and back buffers of the DX11 swapchain backing surface if
  // not null.
  EGLBoolean SwapBuffers(int64_t surface_id);

  // Creates a |EGLSurface| from the provided handle.
  EGLSurface CreateSurfaceFromHandle(EGLenum handle_type,
                                     EGLClientBuffer handle,
                                     const EGLint* attributes) const;

  // Gets the |EGLDisplay|.
  EGLDisplay egl_display() const { return egl_display_; };

  // If enabled, makes the surface's swaps block until the v-blank.
  virtual void SetVSyncEnabled(int64_t surface_id, bool enabled);

  // Gets the |ID3D11Device| chosen by ANGLE.
  bool GetDevice(ID3D11Device** device);

 protected:
  // Creates a new surface manager retaining reference to the passed-in target
  // for the lifetime of the manager.
  explicit AngleSurfaceManager(bool enable_impeller);

 private:
  bool Initialize(bool enable_impeller);
  bool InitializeGlVersion();
  bool InitializeGlExtensions();
  void CleanUp();

  // Attempts to initialize EGL using ANGLE.
  bool InitializeEGL(
      PFNEGLGETPLATFORMDISPLAYEXTPROC egl_get_platform_display_EXT,
      const EGLint* config,
      bool should_log);

  // Whether a render surface exists for the given ID.
  bool RenderSurfaceExists(int64_t surface_id);

  // EGL representation of native display.
  EGLDisplay egl_display_;

  // EGL representation of current rendering context.
  EGLContext egl_context_;

  // EGL representation of current rendering context used for async texture
  // uploads.
  EGLContext egl_resource_context_;

  // current frame buffer configuration.
  EGLConfig egl_config_;

  // The OpenGL major version number.
  int gl_version_major_ = 0;

  // The OpenGL minor version number.
  int gl_version_minor_ = 0;

  // The available OpenGL extensions.
  std::unordered_set<std::string> gl_extensions_;

  // State representing success or failure of display initialization used when
  // creating surfaces.
  bool initialize_succeeded_;

  struct AngleSurface {
    AngleSurface(EGLSurface surface, EGLint width, EGLint height)
        : surface(surface), width(width), height(height) {}
    EGLSurface surface = EGL_NO_SURFACE;
    EGLint width = 0;
    EGLint height = 0;
  };

  // Surfaces the engine can draw into.
  std::unordered_map<int64_t, std::unique_ptr<AngleSurface>> render_surfaces_;

  // The current D3D device.
  Microsoft::WRL::ComPtr<ID3D11Device> resolved_device_;

  // Number of active instances of AngleSurfaceManager
  static int instance_count_;

  FML_DISALLOW_COPY_AND_ASSIGN(AngleSurfaceManager);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_ANGLE_SURFACE_MANAGER_H_
