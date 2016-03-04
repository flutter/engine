// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GL_PROGRAM_H_
#define VFX_GL_PROGRAM_H_

#include <GL/gl.h>

#include "base/macros.h"

namespace vfx {
class Shader;

class Program {
 public:
  Program(Shader* vertex_shader, Shader* fragment_shader);
  ~Program();

  GLuint id() const { return id_; }

 private:
  GLuint id_;

  DISALLOW_COPY_AND_ASSIGN(Program);
};

}  // namespace vfx

#endif  // VFX_GL_PROGRAM_H_
