// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/external_texture_d3d.h"

#include <iostream>

namespace flutter {

struct ExternalTextureD3dState {
  GLuint gl_texture = 0;
  EGLSurface egl_surface = EGL_NO_SURFACE;
  void* last_surface_handle = nullptr;
};

ExternalTextureD3d::ExternalTextureD3d(
    const FlutterDesktopGpuSurfaceTextureCallback texture_callback,
    void* user_data,
    const AngleSurfaceManager* surface_manager,
    const GlProcs& gl_procs)
    : state_(std::make_unique<ExternalTextureD3dState>()),
      texture_callback_(texture_callback),
      user_data_(user_data),
      surface_manager_(surface_manager),
      gl_(gl_procs) {}

ExternalTextureD3d::~ExternalTextureD3d() {
  if (state_->egl_surface != EGL_NO_SURFACE) {
    eglDestroySurface(surface_manager_->egl_display(), state_->egl_surface);
  }

  if (state_->gl_texture != 0) {
    gl_.glDeleteTextures(1, &state_->gl_texture);
  }
}

bool ExternalTextureD3d::PopulateTexture(size_t width,
                                         size_t height,
                                         FlutterOpenGLTexture* opengl_texture) {
  const FlutterDesktopGpuSurfaceDescriptor* descriptor =
      texture_callback_(width, height, user_data_);

  if (!CreateOrUpdateTexture(descriptor)) {
    return false;
  }

  // Populate the texture object used by the engine.
  opengl_texture->target = GL_TEXTURE_2D;
  opengl_texture->name = state_->gl_texture;
  opengl_texture->format = GL_RGBA8;
  opengl_texture->destruction_callback = nullptr;
  opengl_texture->user_data = nullptr;
  opengl_texture->width = descriptor->visible_width;
  opengl_texture->height = descriptor->visible_height;

  return true;
}

bool ExternalTextureD3d::CreateOrUpdateTexture(
    const FlutterDesktopGpuSurfaceDescriptor* descriptor) {
  if (descriptor == nullptr || descriptor->handle == nullptr) {
    return false;
  }

  ExternalTextureD3dState* state = state_.get();
  if (state->gl_texture == 0) {
    gl_.glGenTextures(1, &state_->gl_texture);

    gl_.glBindTexture(GL_TEXTURE_2D, state_->gl_texture);

    gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    gl_.glBindTexture(GL_TEXTURE_2D, state->gl_texture);
  }

  if (descriptor->handle != state->last_surface_handle) {
    if (state->egl_surface != EGL_NO_SURFACE) {
      eglReleaseTexImage(surface_manager_->egl_display(), state->egl_surface,
                         EGL_BACK_BUFFER);
      eglDestroySurface(surface_manager_->egl_display(), state->egl_surface);
    }

    EGLint attributes[] = {EGL_WIDTH,
                           static_cast<EGLint>(descriptor->width),
                           EGL_HEIGHT,
                           static_cast<EGLint>(descriptor->height),
                           EGL_TEXTURE_TARGET,
                           EGL_TEXTURE_2D,
                           EGL_TEXTURE_FORMAT,
                           EGL_TEXTURE_RGBA,  // always EGL_TEXTURE_RGBA
                           EGL_NONE};

    state->last_surface_handle = descriptor->handle;
    state->egl_surface = surface_manager_->CreateSurfaceFromHandle(
        EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE, descriptor->handle, attributes);

    if (state->egl_surface == EGL_NO_SURFACE ||
        eglBindTexImage(surface_manager_->egl_display(), state->egl_surface,
                        EGL_BACK_BUFFER) == EGL_FALSE) {
      std::cerr << "Binding DXGI surface failed." << std::endl;
    }
  }

  return state->egl_surface != EGL_NO_SURFACE;
}

}  // namespace flutter
