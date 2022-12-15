// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/blending.glsl>
#include <impeller/color.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

uniform FragInfo {
  float src_y_coord_scale;
  float alpha;
}
blend_info;

uniform sampler2D texture_sampler_src;

in vec2 v_src_texture_coords;
in vec4 v_dst_color;  // This color input is expected to be unpremultiplied.

out vec4 frag_color;

void main() {
  vec4 dst = v_dst_color;
  vec4 src = IPUnpremultiply(IPSampleWithTileMode(
      texture_sampler_src,           // sampler
      v_src_texture_coords,          // texture coordinates
      blend_info.src_y_coord_scale,  // y coordinate scale
      kTileModeDecal                 // tile mode
      ));

  vec4 blended = vec4(Blend(dst.rgb, src.rgb), 1) * dst.a;
  frag_color = mix(dst, blended, src.a) * blend_info.alpha;
}
