// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/gaussian.glsl>
#include <impeller/types.glsl>

uniform FragInfo {
  vec4 color;
  vec2 rect_size;
  float corner_radius;
}
frag_info;

in vec2 v_position;

out vec4 frag_color;

float RRectDistance(vec2 sample_position, vec2 half_size) {
  vec2 space = abs(sample_position) - half_size + frag_info.corner_radius;
  return length(max(space, 0.0)) + min(max(space.x, space.y), 0.0) -
         frag_info.corner_radius;
}

void main() {
  vec2 half_size = frag_info.rect_size * 0.5;
  vec2 sample_position = v_position - half_size;

  frag_color *= frag_info.color * -RRectDistance(sample_position, half_size);
}