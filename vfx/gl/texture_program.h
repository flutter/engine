// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GL_TEXTURE_PROGRAM_H_
#define VFX_GL_TEXTURE_PROGRAM_H_

#include "base/macros.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gl/gl_bindings.h"
#include "vfx/geometry/color.h"
#include "vfx/geometry/matrix.h"
#include "vfx/geometry/point.h"
#include "vfx/gl/program.h"
#include "vfx/gl/shader.h"

namespace vfx {

class TextureProgram {
 public:
  TextureProgram();
  ~TextureProgram();

  struct Vertex {
    Point point;
    Color color;
    gfx::PointF tex_coord;
  };

  GLuint id() const { return program_.id(); }

  template<typename Buffer>
  void Draw(const Matrix& transform,
            GLuint texture_id,
            const Buffer& geometry) {
    const GLvoid* kPositionOffset = nullptr;
    const GLvoid* kColorOffset = reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 3);
    const GLvoid* kTexCoordOffset =
        reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 7);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glUseProgram(program_.id());
    glUniformMatrix4fv(u_transform_, 1, GL_FALSE, transform.data());
    glUniform1i(u_texture_, 0);
    geometry.Bind();
    glVertexAttribPointer(a_position_, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          kPositionOffset);
    glVertexAttribPointer(a_color_, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          kColorOffset);
    glVertexAttribPointer(a_tex_coord_, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          kTexCoordOffset);
    geometry.Draw();
  }

 private:
  Shader vertex_shader_;
  Shader fragment_shader_;
  Program program_;

  GLint u_transform_;
  GLint u_texture_;
  GLint a_position_;
  GLint a_color_;
  GLint a_tex_coord_;

  DISALLOW_COPY_AND_ASSIGN(TextureProgram);
};

}  // namespace vfx

#endif  // VFX_GL_TEXTURE_PROGRAM_H_
