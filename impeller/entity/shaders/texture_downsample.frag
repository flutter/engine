// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision mediump float;

#include <impeller/constants.glsl>
#include <impeller/types.glsl>

uniform f16sampler2D texture_sampler;

uniform FragInfo {
  float edge;
  float ratio;
  vec2 pixel_size;
}
frag_info;

in highp vec2 v_texture_coords;

out f16vec4 frag_color;

void main() {
  f16vec4 total = f16vec4(0.0hf);
  float16_t ratio = float16_t(frag_info.ratio);
  for (float i = -frag_info.edge; i <= frag_info.edge; i += 2) {
    for (float j = -frag_info.edge; j <= frag_info.edge; j += 2) {
      total += (texture(texture_sampler,
                        v_texture_coords + frag_info.pixel_size * vec2(i, j),
                        float16_t(kDefaultMipBias)) *
                ratio);
    }
  }
  frag_color = total;
}
