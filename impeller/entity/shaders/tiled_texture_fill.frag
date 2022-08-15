// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>
#include <impeller/transform.glsl>

uniform sampler2D texture_sampler;

uniform FragInfo {
  float texture_sampler_y_coord_scale;
  float x_tile_mode;
  float y_tile_mode;
  mat4 matrix;
  vec2 texture_size;
}
frag_info;

in vec2 interpolated_vertices;

out vec4 frag_color;

void main() {
  vec2 transformed_vertices = transform(frag_info.matrix, interpolated_vertices);
  vec2 texture_coords = transformed_vertices / frag_info.texture_size;
  frag_color =
      IPSampleWithTileMode(
          texture_sampler,                          // sampler
          texture_coords,                           // texture coordinates
          frag_info.texture_sampler_y_coord_scale,  // y coordinate scale
          frag_info.x_tile_mode,                    // x tile mode
          frag_info.y_tile_mode                     // y tile mode
      );
}
