// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// 1D (directional) gaussian blur.
//
// Paths for future optimization:
//   * Remove the uv bounds multiplier in SampleColor by adding optional
//     support for SamplerAddressMode::ClampToBorder in the texture sampler.
//   * Render both blur passes into a smaller texture than the source image
//     (~1/radius size).
//   * If doing the small texture render optimization, cache misses can be
//     reduced in the first pass by sampling the source textures with a mip
//     level of log2(min_radius).

#include <impeller/constants.glsl>
#include <impeller/gaussian.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

uniform f16sampler2D texture_sampler;
uniform f16sampler2D alpha_mask_sampler;

uniform FragInfo {
  f16vec2 texture_size;
  f16vec2 blur_direction;

  // The blur sigma and radius have a linear relationship which is defined
  // host-side, but both are useful controls here. Sigma (pixels per standard
  // deviation) is used to define the gaussian function itself, whereas the
  // radius is used to limit how much of the function is integrated.
  float16_t blur_sigma;
  float16_t blur_radius;

  float16_t src_factor;
  float16_t inner_blur_factor;
  float16_t outer_blur_factor;
}
frag_info;

in f16vec2 v_texture_coords;
in f16vec2 v_src_texture_coords;

out f16vec4 frag_color;

void main() {
  f16vec4 total_color = f16vec4(0.0hf);
  float16_t gaussian_integral = 0.0hf;
  f16vec2 blur_uv_offset = frag_info.blur_direction / frag_info.texture_size;

  for (float16_t i = -frag_info.blur_radius; i <= frag_info.blur_radius; i++) {
    float16_t gaussian = IPGaussian(i, frag_info.blur_sigma);
    gaussian_integral += gaussian;
    total_color +=
        gaussian *
        Sample(texture_sampler,                       // sampler
               v_texture_coords + blur_uv_offset * i  // texture coordinates
        );
  }

  f16vec4 blur_color = total_color / gaussian_integral;

  f16vec4 src_color = Sample(alpha_mask_sampler,   // sampler
                             v_src_texture_coords  // texture coordinates
  );
  float16_t blur_factor =
      frag_info.inner_blur_factor * float16_t(src_color.a > 0.0hf) +
      frag_info.outer_blur_factor * float16_t(src_color.a == 0.0hf);

  frag_color = blur_color * blur_factor + src_color * frag_info.src_factor;
}
