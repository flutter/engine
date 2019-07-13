// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_

#include "flutter/shell/platform/embedder/embedder.h"

#include <stdint.h>
#include <memory>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/texture_registrar.h"
#include "flutter/shell/platform/common/cpp/public/flutter_texture_registrar.h"
#include "flutter/shell/platform/glfw/public/flutter_glfw.h"

typedef unsigned int GLuint;
typedef struct GLFWwindow GLFWwindow;

namespace flutter {

class ExternalTextureGL {
 public:
  ExternalTextureGL(FlutterTexutreCallback texture_callback, void* user_data);

  virtual ~ExternalTextureGL();

  int64_t texutre_id() { return reinterpret_cast<int64_t>(this); }

  virtual bool PopulateTextureWithIdentifier(size_t width, size_t height,
                                             FlutterOpenGLTexture* texture);

 private:
  FlutterTexutreCallback texture_callback_ = nullptr;
  void* user_data_ = nullptr;
  GLuint glTexture = 0;
  GLFWwindow* window_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_
