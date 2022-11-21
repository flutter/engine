// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>
#include <impeller/gaussian.glsl>
#include <impeller/texture.glsl>

// Constant time mask blur for image borders.
//
// This mask blur extends the geometry of the source image (with clamp border
// sampling) and applies a Gaussian blur to the alpha mask at the edges.
//
// The blur itself works by mapping the Gaussian distribution's indefinite
// integral (using an erf approximation) to the 4 edges of the UV rectangle and
// multiplying them.

uniform sampler2D texture_sampler;

uniform FragInfo {
  float16_t texture_sampler_y_coord_scale;
} frag_info;

in f16vec2 v_texture_coords;
in f16vec2 v_sigma_uv;
in float16_t v_src_factor;
in float16_t v_inner_blur_factor;
in float16_t v_outer_blur_factor;

out f16vec4 frag_color;

float16_t BoxBlurMask(f16vec2 uv) {
  // LTRB
  return IPGaussianIntegral(uv.x, v_sigma_uv.x) *      //
         IPGaussianIntegral(uv.y, v_sigma_uv.y) *      //
         IPGaussianIntegral(1.0hf - uv.x, v_sigma_uv.x) *  //
         IPGaussianIntegral(1.0hf - uv.y, v_sigma_uv.y);
}

void main() {
  f16vec4 image_color = IPSample(texture_sampler, v_texture_coords,
                              frag_info.texture_sampler_y_coord_scale);
  float16_t blur_factor = BoxBlurMask(v_texture_coords);

  float16_t within_bounds =
      float16_t(v_texture_coords.x >= 0.0hf && v_texture_coords.y >= 0.0hf &&
            v_texture_coords.x < 1.0hf && v_texture_coords.y < 1.0hf);
  float16_t inner_factor =
      (v_inner_blur_factor * blur_factor + v_src_factor) * within_bounds;
  float16_t outer_factor = v_outer_blur_factor * blur_factor * (1.0hf - within_bounds);

  float16_t mask_factor = inner_factor + outer_factor;
  frag_color = image_color * mask_factor;
}
