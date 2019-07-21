// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_

#include <stdint.h>

#include "flutter/shell/platform/common/cpp/public/flutter_texture_registrar.h"
#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

typedef struct ExternalTextureGLState ExternalTextureGLState;

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
  ExternalTextureGLState* state_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_
