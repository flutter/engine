// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>

uniform sampler2D texture_sampler;

uniform GradientInfo {
  vec2 center;
  float radius;
  float tile_mode;
  float texture_sampler_y_coord_scale;
} gradient_info;

in vec2 v_position;

out vec4 frag_color;

void main() {
  float len = length(v_position - gradient_info.center);
  float t = len / gradient_info.radius;
  if ((t < 0.0 || t > 1.0) && gradient_info.tile_mode == kTileModeDecal) {
    frag_color = vec4(0);
    return;
  }

  t = IPFloatTile(t, gradient_info.tile_mode);
  frag_color = IPSample(texture_sampler, vec2(t, 0.5), gradient_info.texture_sampler_y_coord_scale);
}
