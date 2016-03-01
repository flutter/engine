// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/gl/texture_program.h"

#include <memory>

#include "ui/gl/gl_bindings.h"

namespace vfx {
namespace {

const char kVertexShaderSource[] = R"GLSL(
uniform mat4 u_transform;
attribute vec3 a_position;
attribute vec4 a_color;
attribute vec2 a_tex_coord;

varying vec2 v_tex_coord;
varying vec4 v_color;

void main() {
  gl_Position = u_transform * vec4(a_position, 1.0);
  v_color = a_color;
  v_tex_coord = a_tex_coord;
}
)GLSL";


const char kFragmentShaderSource[] = R"GLSL(
varying lowp vec4 v_color;
varying lowp vec2 v_tex_coord;
uniform sampler2D u_texture;

void main(void) {
  gl_FragColor = v_color * texture2D(u_texture, v_tex_coord);
}
)GLSL";

}  // namespace

TextureProgram::TextureProgram()
  : vertex_shader_(GL_VERTEX_SHADER, kVertexShaderSource),
    fragment_shader_(GL_FRAGMENT_SHADER, kFragmentShaderSource),
    program_(&vertex_shader_, &fragment_shader_),
    u_transform_(glGetUniformLocation(program_.id(), "u_transform")),
    u_texture_(glGetUniformLocation(program_.id(), "u_texture")),
    a_position_(glGetAttribLocation(program_.id(), "a_position")),
    a_color_(glGetAttribLocation(program_.id(), "a_color")),
    a_tex_coord_(glGetAttribLocation(program_.id(), "a_tex_coord")) {
  glEnableVertexAttribArray(a_position_);
  glEnableVertexAttribArray(a_color_);
  glEnableVertexAttribArray(a_tex_coord_);
}

TextureProgram::~TextureProgram() {
}

}  // namespace vfx
