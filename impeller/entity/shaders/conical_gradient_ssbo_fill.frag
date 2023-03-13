// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/gradient.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

struct ColorPoint {
  vec4 color;
  float stop;
};

layout(std140) readonly buffer ColorData {
  ColorPoint colors[];
}
color_data;

uniform FragInfo {
  vec2 center;
  float radius;
  float tile_mode;
  float alpha;
  int colors_length;
  vec2 focus;
  float focus_radius;
}
frag_info;

in vec2 v_position;

out vec4 frag_color;

float calc_t(vec2 c0, float r0, vec2 c1, float r1, vec2 p) {
  float w = 1.0;
  float result = 0.0;
  vec2 ab = c1 - c0;
  float dr = r1 - r0;
  float delta = 1.0 / length(ab);
  while (w >= 0.0) {
    vec2 cw = w * ab + c0;
    float rw = w * dr + r0;
    if (length(p - cw) <= rw) {
      result = w;
      break;
    }
    w -= delta;
  }
  return 1.0 - result;
}

void main() {
  float t = calc_t(frag_info.center, frag_info.radius, frag_info.focus,
                   frag_info.focus_radius, v_position);

  if ((t < 0.0 || t > 1.0) && frag_info.tile_mode == kTileModeDecal) {
    frag_color = vec4(0);
    return;
  }
  t = IPFloatTile(t, frag_info.tile_mode);

  vec4 result_color = vec4(0);
  for (int i = 1; i < frag_info.colors_length; i++) {
    ColorPoint prev_point = color_data.colors[i - 1];
    ColorPoint current_point = color_data.colors[i];
    if (t >= prev_point.stop && t <= current_point.stop) {
      float delta = (current_point.stop - prev_point.stop);
      if (delta < 0.001) {
        result_color = current_point.color;
      } else {
        float ratio = (t - prev_point.stop) / delta;
        result_color = mix(prev_point.color, current_point.color, ratio);
      }
      break;
    }
  }
  frag_color =
      vec4(result_color.xyz * result_color.a, result_color.a) * frag_info.alpha;
}
