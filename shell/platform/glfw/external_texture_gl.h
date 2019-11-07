// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_

#include <stdint.h>
#include <memory>

#include "flutter/shell/platform/common/cpp/public/flutter_texture_registrar.h"
#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

typedef struct ExternalTextureGLState ExternalTextureGLState;

// An adaptation class of flutter engine and external texture interface.
class ExternalTextureGL {
 public:
  ExternalTextureGL(FlutterTexutreCallback texture_callback, void* user_data);

  virtual ~ExternalTextureGL();

  /**
   * Returns the unique id for the ExternalTextureGL instance.
   */
  int64_t texture_id() { return reinterpret_cast<int64_t>(this); }

  /**
   * Accepts texture buffer copy request from the Flutter engine.
   * When the user side marks the texture_id as available, the Flutter engine
   * will callback to this method and ask for populate the |opengl_texture|
   * object, such as the texture type and the format of the pixel buffer and the
   * texture object.
   * Returns true on success, false on failure.
   */
  bool PopulateTextureWithIdentifier(size_t width,
                                     size_t height,
                                     FlutterOpenGLTexture* opengl_texture);

 private:
  std::unique_ptr<ExternalTextureGLState> state_;
  FlutterTexutreCallback texture_callback_ = nullptr;
  void* user_data_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_GLFW_EXTERNAL_TEXTURE_GL_H_
