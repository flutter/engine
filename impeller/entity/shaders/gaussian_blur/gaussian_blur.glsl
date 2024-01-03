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

struct BlurSample {
  f16vec2 uv_offset;
  float16_t coefficient;
};

uniform BlurInfo {
  int sample_count;
  BlurSample samples[24];
}
blur_info;

f16vec4 Sample(f16sampler2D tex, vec2 coords) {
#if ENABLE_DECAL_SPECIALIZATION
  return IPHalfSampleDecal(tex, coords);
#else
  return texture(tex, coords);
#endif
}

in vec2 v_texture_coords;

out f16vec4 frag_color;

void main() {
  f16vec4 total_color = f16vec4(0.0hf);
  float16_t gaussian_integral = 0.0hf;

  for (int i = 0; i < blur_info.sample_count; ++i) {
    float16_t coefficient = blur_info.samples[i].coefficient;
    gaussian_integral += coefficient;
    total_color +=
        coefficient * Sample(texture_sampler,
                             v_texture_coords + blur_info.samples[i].uv_offset);
  }

  frag_color = total_color / gaussian_integral;
}
