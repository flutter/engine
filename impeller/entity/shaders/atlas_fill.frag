// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>
#include <impeller/types.glsl>

uniform FragInfo {
  float src_y_coord_scale;
  float alpha;
}
frag_info;

uniform sampler2D texture_sampler_src;

in vec2 v_src_texture_coords;
in vec4 v_dst_color;  // This color input is expected to be unpremultiplied.

out vec4 frag_color;

void main() {
  vec4 sampled = IPSample(texture_sampler_src, v_src_texture_coords,
                          frag_info.src_y_coord_scale);
  frag_color = sampled * v_dst_color * frag_info.alpha;
}
