// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/color.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

// Creates a color filter that applies the inverse of the sRGB gamma curve
// to the RGB channels.

uniform sampler2D input_texture;

uniform FragInfo {
<<<<<<< HEAD
  float16_t texture_sampler_y_coord_scale;
  float16_t input_alpha;
}
frag_info;
=======
  float texture_sampler_y_coord_scale;
  float input_alpha;
}
frag_info;
>>>>>>> ddf6a20b86578f147ee7da023f3f08ecb4256d07

in f16vec2 v_position;
out f16vec4 frag_color;

void main() {
  f16vec4 input_color = IPSample(input_texture, v_position,
                                 frag_info.texture_sampler_y_coord_scale) *
                        frag_info.input_alpha;

  f16vec4 color = IPUnpremultiply(input_color);
  for (int i = 0; i < 3; i++) {
    if (color[i] <= 0.04045hf) {
      color[i] = color[i] / 12.92hf;
    } else {
      color[i] = pow((color[i] + 0.055hf) / 1.055hf, 2.4hf);
    }
  }

  frag_color = IPPremultiply(color);
}
