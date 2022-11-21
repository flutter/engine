// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/gradient.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

readonly buffer ColorData {
<<<<<<< HEAD
  f16vec4 colors[];
}
color_data;

uniform GradientInfo {
  f16vec2 start_point;
  f16vec2 end_point;
  float16_t alpha;
  float16_t tile_mode;
  float16_t colors_length;
}
gradient_info;
=======
  vec4 colors[];
}
color_data;

uniform GradientInfo {
  vec2 start_point;
  vec2 end_point;
  float alpha;
  float tile_mode;
  float colors_length;
}
gradient_info;
>>>>>>> ddf6a20b86578f147ee7da023f3f08ecb4256d07

in f16vec2 v_position;

out f16vec4 frag_color;

void main() {
<<<<<<< HEAD
  float16_t len = length(gradient_info.end_point - gradient_info.start_point);
  float16_t dot = dot(v_position - gradient_info.start_point,
                      gradient_info.end_point - gradient_info.start_point);
  float16_t t = dot / (len * len);
=======
  float len = length(gradient_info.end_point - gradient_info.start_point);
  float dot = dot(v_position - gradient_info.start_point,
                  gradient_info.end_point - gradient_info.start_point);
  float t = dot / (len * len);
>>>>>>> ddf6a20b86578f147ee7da023f3f08ecb4256d07

  if ((t < 0.0hf || t > 1.0hf) && gradient_info.tile_mode == kTileModeDecal) {
    frag_color = f16vec4(0.0hf);
    return;
  }
  t = IPFloatTile(t, gradient_info.tile_mode);
  f16vec3 values = IPComputeFixedGradientValues(t, gradient_info.colors_length);

<<<<<<< HEAD
  frag_color = mix(color_data.colors[int(values.x)],
                   color_data.colors[int(values.y)], values.z);
  frag_color = f16vec4(frag_color.xyz * frag_color.a, frag_color.a) *
               gradient_info.alpha;
=======
  frag_color = mix(color_data.colors[int(values.x)],
                   color_data.colors[int(values.y)], values.z);
  frag_color =
      vec4(frag_color.xyz * frag_color.a, frag_color.a) * gradient_info.alpha;
>>>>>>> ddf6a20b86578f147ee7da023f3f08ecb4256d07
}
