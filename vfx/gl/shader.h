// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GL_SHADER_H_
#define VFX_GL_SHADER_H_

#include <GL/gl.h>
#include <string>

#include "base/macros.h"

namespace vfx {

class Shader {
 public:
  Shader(GLenum type, const std::string& source);
  ~Shader();

  GLenum type() const { return type_; }
  GLuint id() const { return id_; }

 private:
  GLenum type_;
  GLuint id_;

  DISALLOW_COPY_AND_ASSIGN(Shader);
};

}  // namespace vfx

#endif  // VFX_GL_SHADER_H_
