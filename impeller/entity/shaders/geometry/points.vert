// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>
#include <impeller/types.glsl>

uniform FrameInfo {
  float16_t radius;
  float16_t radian_start;
  float16_t radian_step;
  int points_per_circle;
  int divisions_per_circle;
}
frame_info;

layout(std430) writeonly buffer GeometryData {
  vec2 geometry[];
}
geometry_data;

in vec2 center;

void main() {
  // The buffer offset we start writing to is the number of data per circle *
  // number of previous circles.
  int bufer_offset = gl_VertexIndex * frame_info.points_per_circle;

  float16_t elapsed_angle = frame_info.radian_start;

  vec2 origin =
      center + vec2(cos(elapsed_angle), sin(elapsed_angle)) * frame_info.radius;
  geometry_data.geometry[bufer_offset++] = origin;

  elapsed_angle += frame_info.radian_step;
  vec2 pt1 =
      center + vec2(cos(elapsed_angle), sin(elapsed_angle)) * frame_info.radius;
  geometry_data.geometry[bufer_offset++] = pt1;

  elapsed_angle += frame_info.radian_step;
  vec2 pt2 =
      center + vec2(cos(elapsed_angle), sin(elapsed_angle)) * frame_info.radius;
  geometry_data.geometry[bufer_offset++] = pt2;

  for (int i = 0; i < frame_info.divisions_per_circle - 2; i++) {
    geometry_data.geometry[bufer_offset++] = origin;
    geometry_data.geometry[bufer_offset++] = pt2;

    elapsed_angle += frame_info.radian_step;
    pt2 = center +
          vec2(cos(elapsed_angle), sin(elapsed_angle)) * frame_info.radius;
    geometry_data.geometry[bufer_offset++] = pt2;
  }
}
