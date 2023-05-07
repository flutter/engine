// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>
#include <impeller/types.glsl>

// TODO(jonahwilliams): with some adjustments to start and end angles,
// this could draw squars and closed arcs too.

uniform FrameInfo {
  float radius;
  float16_t radian_step;
  int points_per_circle;
  int divisions_per_circle;
  int total_length;
}
frame_info;

layout(std430) writeonly buffer GeometryData {
  vec2 geometry[];
}
geometry_data;

in vec2 center;
in int offset;

void main() {
  // The buffer offset we start writing to is the number of data per circle *
  // number of previous circles.
  int bufer_offset = offset * frame_info.points_per_circle;

  float16_t elapsed_angle = 0.0hf;
  geometry_data.geometry[bufer_offset++] = center;

  vec2 pt1 = center + vec2(1, 0) * frame_info.radius;
  geometry_data.geometry[bufer_offset++] = pt1;

  elapsed_angle += frame_info.radian_step;
  vec2 pt2 =
      center + vec2(cos(elapsed_angle), sin(elapsed_angle)) * frame_info.radius;
  geometry_data.geometry[bufer_offset++] = pt2;

  for (int i = 1; i < frame_info.divisions_per_circle; i++) {
    geometry_data.geometry[bufer_offset++] = center;

    pt1 = pt2;
    elapsed_angle += frame_info.radian_step;
    geometry_data.geometry[bufer_offset++] = pt1;

    pt2 = center +
          vec2(cos(elapsed_angle), sin(elapsed_angle)) * frame_info.radius;
    geometry_data.geometry[bufer_offset++] = pt2;
  }
}
