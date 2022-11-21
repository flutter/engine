// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>
#include <impeller/gradient.glsl>
#include <impeller/texture.glsl>

readonly buffer ColorData {
  f16vec4 colors[];
} color_data;

uniform GradientInfo {
  f16vec2 center;
  float16_t radius;
  float16_t tile_mode;
  float16_t alpha;
  float16_t colors_length;
} gradient_info;

in f16vec2 v_position;

out f16vec4 frag_color;

void main() {
  float16_t len = length(v_position - gradient_info.center);
  float16_t t = len / gradient_info.radius;

  if ((t < 0.0hf || t > 1.0hf) && gradient_info.tile_mode == kTileModeDecal) {
    frag_color = f16vec4(0.0hf);
    return;
  }
  t = IPFloatTile(t, gradient_info.tile_mode);
  f16vec3 values = IPComputeFixedGradientValues(t, gradient_info.colors_length);

  frag_color = mix(color_data.colors[int(values.x)], color_data.colors[int(values.y)], values.z);
  frag_color = f16vec4(frag_color.xyz * frag_color.a, frag_color.a) * gradient_info.alpha;
}
