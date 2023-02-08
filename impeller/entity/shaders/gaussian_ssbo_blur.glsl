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

uniform sampler2D texture_sampler;
uniform sampler2D alpha_mask_sampler;

struct KernelData {
  float16_t texture_coord_offset_x;
  float16_t texture_coord_offset_y;
  float16_t gaussian;
};

layout(std140) readonly buffer GaussianKernel {
  KernelData data[];
}
gaussian_kernel;

uniform FragInfo {
  float16_t sample_count;
  float16_t gaussian_integral;
  float16_t src_factor;
  float16_t inner_blur_factor;
  float16_t outer_blur_factor;
}
frag_info;

in vec2 v_texture_coords;
in vec2 v_src_texture_coords;

out vec4 frag_color;

void main() {
  f16vec4 total_color = f16vec4(0);
  f16vec2 texture_coords = f16vec2(v_texture_coords);
  f16vec2 src_texture_coords = f16vec2(v_src_texture_coords);
  int total = int(frag_info.sample_count);
  for (int i = 0; i <= total; i++) {
    KernelData data = gaussian_kernel.data[i];
    total_color +=
        data.gaussian *
        Sample(texture_sampler,  // sampler
               texture_coords +
                   f16vec2(data.texture_coord_offset_x,
                           data.texture_coord_offset_y)  // texture coordinates
        );
  }

  f16vec4 blur_color = total_color / frag_info.gaussian_integral;

  f16vec4 src_color = Sample(alpha_mask_sampler,   // sampler
                             src_texture_coords  // texture coordinates
  );
  float blur_factor = frag_info.inner_blur_factor * float(src_color.a > 0.0f) +
                      frag_info.outer_blur_factor * float(src_color.a == 0.0f);

  frag_color = blur_color * blur_factor + src_color * frag_info.src_factor;
}
