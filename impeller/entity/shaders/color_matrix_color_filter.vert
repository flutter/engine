// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

uniform FrameInfo {
  mat4 mvp;

  mat4 m;
  vec4 v;
} frame_info;

in vec2 vertices;
in vec4 vertex_color;

out vec4 v_color;
out mat4 v_m;
out vec4 v_v;

void main() {
  gl_Position = frame_info.mvp * vec4(vertices, 0.0, 1.0);
  v_m = frame_info.m;
  v_v = frame_info.v;
  v_color = vertex_color;
}
