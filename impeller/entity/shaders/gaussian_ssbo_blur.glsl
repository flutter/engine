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

struct KernelData {
  vec2 texture_coord_offset;
  float gaussian;
};

// layout(std140) readonly buffer GaussianKernel {
//   KernelData data[];
// }
// gaussian_kernel;

uniform FragInfo {
  float sample_count;
  KernelData data[16];
}
frag_info;

in vec2 v_texture_coords;
in vec2 v_src_texture_coords;

out vec4 frag_color;

void main() {
  vec4 total_color = f16vec4(0);
  int total = int(frag_info.sample_count);
  for (int i = 0; i <= total; i++) {
    KernelData data = frag_info.data[i];
    total_color += data.gaussian *
                   Sample(texture_sampler,  // sampler
                          v_texture_coords +
                              data.texture_coord_offset  // texture coordinates
                   );
  }

  frag_color = total_color;
}
