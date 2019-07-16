// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/external_texture_gl.h"

namespace flutter {

static void OnGLBufferRelease(void* user_data) {}

ExternalTextureGL::ExternalTextureGL(FlutterTexutreCallback texture_callback,
                                     void* user_data)
    : texture_callback_(texture_callback), user_data_(user_data) {}

ExternalTextureGL::~ExternalTextureGL() {
  if (gl_texture_ != 0)
    glDeleteTextures(1, &gl_texture_);
}

int64_t ExternalTextureGL::texutre_id() {
  return reinterpret_cast<int64_t>(this);
}

bool ExternalTextureGL::PopulateTextureWithIdentifier(
    size_t width,
    size_t height,
    FlutterOpenGLTexture* texture) {
  // Confirm that the current window context is available.
  if (!window_) {
    window_ = glfwGetCurrentContext();
    if (window_) {
      glfwMakeContextCurrent(window_);
      // Load glad functions.
      gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    }
  }

  if (!window_)
    return false;

  const PixelBuffer* pixel_buffer =
      texture_callback_(width, height, user_data_);

  if (!pixel_buffer || !pixel_buffer->buffer)
    return false;

  if (gl_texture_ == 0) {
    glGenTextures(1, &gl_texture_);
    glBindTexture(GL_TEXTURE_2D, gl_texture_);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  glBindTexture(GL_TEXTURE_2D, gl_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixel_buffer->width,
               pixel_buffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               pixel_buffer->buffer);

  texture->target = GL_TEXTURE_2D;
  texture->name = gl_texture_;
  texture->format = GL_RGBA8;
  texture->destruction_callback = (VoidCallback)&OnGLBufferRelease;
  texture->user_data = (void*)this;
  return true;
}

}  // namespace flutter
