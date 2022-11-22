// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/constants.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

// These values must correspond to the order of the items in the
// 'FilterContents::MorphType' enum class.
const float16_t kMorphTypeDilate = 0.0hf;
const float16_t kMorphTypeErode = 1.0hf;

uniform sampler2D texture_sampler;

uniform FragInfo {
  float16_t texture_sampler_y_coord_scale;
  f16vec2 texture_size;
  f16vec2 direction;
  float16_t radius;
  float16_t morph_type;
}
frag_info;

in f16vec2 v_texture_coords;
out f16vec4 frag_color;

void main() {
  f16vec4 result =
      frag_info.morph_type == kMorphTypeDilate ? f16vec4(0) : f16vec4(1);
  f16vec2 uv_offset = frag_info.direction / frag_info.texture_size;
  for (float16_t i = -frag_info.radius; i <= frag_info.radius; i++) {
    f16vec2 texture_coords = v_texture_coords + uv_offset * i;
    f16vec4 color;
    color = IPSampleWithTileMode(
        texture_sampler,                          // sampler
        texture_coords,                           // texture coordinates
        frag_info.texture_sampler_y_coord_scale,  // y coordinate scale
        kTileModeDecal                            // tile mode
    );
    if (frag_info.morph_type == kMorphTypeDilate) {
      result = max(color, result);
    } else {
      result = min(color, result);
    }
  }

  frag_color = result;
}
