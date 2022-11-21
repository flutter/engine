// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>
#include <impeller/texture.glsl>

uniform sampler2D texture_sampler;

uniform FragInfo {
  float16_t texture_sampler_y_coord_scale;
  float16_t x_tile_mode;
  float16_t y_tile_mode;
  float16_t alpha;
}
frag_info;

in f16vec2 v_texture_coords;

out f16vec4 frag_color;

void main() {
  frag_color =
      IPSampleWithTileMode(
          texture_sampler,                          // sampler
          v_texture_coords,                         // texture coordinates
          frag_info.texture_sampler_y_coord_scale,  // y coordinate scale
          frag_info.x_tile_mode,                    // x tile mode
          frag_info.y_tile_mode                     // y tile mode
          ) *
      frag_info.alpha;
}
