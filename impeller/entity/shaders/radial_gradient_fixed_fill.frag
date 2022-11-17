// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/gradient.glsl>

uniform GradientInfo {
  vec2 center;
  float radius;
  float tile_mode;
  float alpha;
  float colors_length;
  vec4 colors[FIXED_GRADIENT_SIZE];
} gradient_info;

in vec2 v_position;

out vec4 frag_color;

void main() {
  float len = length(v_position - gradient_info.center);
  float t = len / gradient_info.radius;
  frag_color = IPComputeFixedGradient(
    t,
    gradient_info.colors,
    gradient_info.colors_length,
    gradient_info.tile_mode);
  frag_color = vec4(frag_color.xyz * frag_color.a, frag_color.a) * gradient_info.alpha;
}
