// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/gl/shader.h"

#include <memory>

#include "base/logging.h"
#include "ui/gl/gl_bindings.h"

namespace vfx {

Shader::Shader(GLenum type, const std::string& source)
  : type_(type), id_(glCreateShader(type)) {
  const char* cstring = source.c_str();
  glShaderSource(id_, 1, &cstring, NULL);
  glCompileShader(id_);

  GLint status = 0;
  glGetShaderiv(id_, GL_COMPILE_STATUS, &status);

  if (!status) {
    GLsizei expected_length = 0;
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &expected_length);
    std::string error;
    error.resize(expected_length);
    GLsizei actual_length = 0;
    glGetShaderInfoLog(id_, expected_length, &actual_length, &error[0]);
    error.resize(actual_length);
    LOG(FATAL) << "Compilation of shader failed: " << error;
    glDeleteShader(id_);
    id_ = 0;
  }
}

Shader::~Shader() {
  if (id_)
    glDeleteShader(id_);
  id_ = 0;
}

}  // namespace vfx
