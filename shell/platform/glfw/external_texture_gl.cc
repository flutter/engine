// Copyright 2018 Google LLC
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/external_texture_gl.h"

#include "GLFW/glfw3.h"
#include "glad/glad.h"

namespace flutter {

static void OnGLBufferRelease(void* user_data) {}

ExternalTextureGL::ExternalTextureGL(FlutterTexutreCallback texture_callback,
                                     void* user_data)
    : texture_callback_(texture_callback), user_data_(user_data) {}

ExternalTextureGL::~ExternalTextureGL() {
  if (glTexture != 0) glDeleteTextures(1, &glTexture);
}

bool ExternalTextureGL::PopulateTextureWithIdentifier(
    size_t width, size_t height, FlutterOpenGLTexture* texture) {
  if (!window_) {
    window_ = glfwGetCurrentContext();
    if (window_) {
      glfwMakeContextCurrent(window_);
      gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    }
  } else {
    glfwMakeContextCurrent(window_);
  }

  if (!window_) return false;

  std::shared_ptr<GLFWPixelBuffer> pixel_buffer =
      texture_callback_(width, height, user_data_);

  if (!pixel_buffer.get()) return false;

  if (glTexture == 0) {
    glGenTextures(1, &glTexture);
    glBindTexture(GL_TEXTURE_2D, glTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  /*Fill texture.*/
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, glTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixel_buffer->width,
                 pixel_buffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixel_buffer->buffer.get());
    glEnable(GL_TEXTURE_2D);
  }
  texture->target = GL_TEXTURE_2D;
  texture->name = glTexture;
  texture->format = GL_RGBA8;
  texture->destruction_callback = (VoidCallback)&OnGLBufferRelease;
  texture->user_data = (void*)this;
  return true;
}

}  // namespace flutter
