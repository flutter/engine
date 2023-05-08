// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>
#include <impeller/types.glsl>

uniform FrameInfo {
  f16vec2 center;
}
frame_info;

layout(std430) writeonly buffer GeometryData {
  vec2 geometry[];
}
geometry_data;

in f16vec2 p1;
in f16vec2 p2;

void main() {
  int bufer_offset = gl_VertexIndex * 3;

  geometry_data[bufer_offset++] = p1;
  geometry_data[bufer_offset++] = p2;
  geometry_data[bufer_offset++] = vec2(frame_info.center);
}
