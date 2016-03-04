// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GL_COLOR_PROGRAM_H_
#define VFX_GL_COLOR_PROGRAM_H_

#include "base/macros.h"
#include "ui/gl/gl_bindings.h"
#include "vfx/geometry/color.h"
#include "vfx/geometry/matrix.h"
#include "vfx/geometry/point.h"
#include "vfx/gl/program.h"
#include "vfx/gl/shader.h"

namespace vfx {

class ColorProgram {
 public:
  ColorProgram();
  ~ColorProgram();

  struct Vertex {
    Point point;
    Color color;
  };

  GLuint id() const { return program_.id(); }

  template<typename Buffer>
  void Draw(const Matrix& transform, const Buffer& geometry) {
    const GLvoid* kPositionOffset = nullptr;
    const GLvoid* kColorOffset = reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 3);

    glUseProgram(program_.id());
    glUniformMatrix4fv(u_transform_, 1, GL_FALSE, transform.data());
    geometry.Bind();
    glVertexAttribPointer(a_position_, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          kPositionOffset);
    glVertexAttribPointer(a_color_, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          kColorOffset);
    geometry.Draw();
  }

 private:
  Shader vertex_shader_;
  Shader fragment_shader_;
  Program program_;

  GLint u_transform_;
  GLint a_position_;
  GLint a_color_;

  DISALLOW_COPY_AND_ASSIGN(ColorProgram);
};

}  // namespace vfx

#endif  // VFX_GL_COLOR_PROGRAM_H_
