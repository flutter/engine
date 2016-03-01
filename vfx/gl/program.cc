// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/gl/program.h"

#include "base/logging.h"
#include "ui/gl/gl_bindings.h"
#include "vfx/gl/shader.h"

namespace vfx {

Program::Program(Shader* vertex_shader, Shader* fragment_shader)
  : id_(glCreateProgram()) {
  DCHECK(vertex_shader->type() == GL_VERTEX_SHADER);
  DCHECK(fragment_shader->type() == GL_FRAGMENT_SHADER);

  glAttachShader(id_, vertex_shader->id());
  glAttachShader(id_, fragment_shader->id());
  glLinkProgram(id_);

  GLint status = 0;
  glGetProgramiv(id_, GL_LINK_STATUS, &status);
  if (!status) {
    GLsizei expected_length = 0;
    glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &expected_length);
    std::string error;
    error.resize(expected_length);
    GLsizei actual_length = 0;
    glGetProgramInfoLog(id_, expected_length, &actual_length, &error[0]);
    error.resize(actual_length);
    LOG(FATAL) << "Linking program failed: " << error;
    glDeleteProgram(id_);
    id_ = 0;
  }
}

Program::~Program() {
  if (id_)
    glDeleteProgram(id_);
  id_ = 0;
}

}  // namespace vfx
