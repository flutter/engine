// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

uniform VertexInfo {
  mat4 mvp;
}
vertex_info;

in vec2 position;
in vec2 border;

out vec2 v_border;

void main() {
  gl_Position = vertex_info.mvp * vec4(position, 0.0, 1.0);
  v_border = border;
}
