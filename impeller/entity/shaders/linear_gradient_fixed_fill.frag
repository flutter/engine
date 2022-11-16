// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>

uniform GradientInfo {
  vec2 start_point;
  vec2 end_point;
  float alpha;
  float tile_mode;
  float colors_length;
  vec4 colors[16];
} gradient_info;

in vec2 v_position;

out vec4 frag_color;

void main() {
  float len = length(gradient_info.end_point - gradient_info.start_point);
  float dot = dot(
    v_position - gradient_info.start_point,
    gradient_info.end_point - gradient_info.start_point
  );
  float t = dot / (len * len);
  if ((t < 0.0 || t > 1.0) && gradient_info.tile_mode == kTileModeDecal) {
    frag_color = vec4(0);
    return;
  }

  t = IPFloatTile(t, gradient_info.tile_mode);
  if (gradient_info.colors_length == 2) {
    frag_color = mix(gradient_info.colors[0], gradient_info.colors[1], t);
    return;
  }
  if (t == 1.0) {
    frag_color = gradient_info.colors[int(gradient_info.colors_length) - 1];
    return;
  }

  float rough_index =  gradient_info.colors_length * t;
  float lower_index = floor(rough_index);
  float scale = rough_index - lower_index;

  frag_color = mix(gradient_info.colors[int(lower_index)], gradient_info.colors[int(lower_index + 1)], scale);
}
