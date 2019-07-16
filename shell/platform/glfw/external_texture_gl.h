// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_

#include "flutter/shell/platform/embedder/embedder.h"

#include <stdint.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/texture_registrar.h"
#include "flutter/shell/platform/glfw/public/flutter_glfw.h"

namespace flutter {

// An adaptation class of flutter engine and external texture interface.
class ExternalTextureGL {
 public:
  ExternalTextureGL(FlutterTexutreCallback texture_callback, void* user_data);

  virtual ~ExternalTextureGL();
 
  int64_t texutre_id();

  bool PopulateTextureWithIdentifier(size_t width,
                                     size_t height,
                                     FlutterOpenGLTexture* texture);

 private:
  FlutterTexutreCallback texture_callback_ = nullptr;
  void* user_data_ = nullptr;
  GLFWwindow* window_ = nullptr;
  GLuint gl_texture_ = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_
