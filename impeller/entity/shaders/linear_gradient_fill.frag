// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>

uniform sampler2D texture_sampler;

uniform GradientInfo {
  vec2 start_point;
  vec2 end_point;
  float tile_mode;
  float texture_sampler_y_coord_scale;
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
  if (gradient_info.tile_mode == kTileModeDecal && (t < 0.0 || t > 1.0)) {
    frag_color = vec4(0);
    return;
  }
  t = IPFloatTile(t, gradient_info.tile_mode);

  frag_color = IPSample(texture_sampler, vec2(t, 0.5), gradient_info.texture_sampler_y_coord_scale);
}
