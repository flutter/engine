// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>
#include <impeller/blending.glsl>
#include <impeller/color.glsl>
#include <impeller/texture.glsl>

uniform BlendInfo {
  float16_t dst_input_alpha;
  float16_t dst_y_coord_scale;
  float16_t src_y_coord_scale;
  float16_t color_factor;
  f16vec4 color;  // This color input is expected to be unpremultiplied.
} blend_info;

uniform sampler2D texture_sampler_dst;
uniform sampler2D texture_sampler_src;

in f16vec2 v_dst_texture_coords;
in f16vec2 v_src_texture_coords;

out f16vec4 frag_color;

void main() {
  f16vec4 dst_sample =
      IPSampleWithTileMode(texture_sampler_dst,           // sampler
                           v_dst_texture_coords,          // texture coordinates
                           blend_info.dst_y_coord_scale,  // y coordinate scale
                           kTileModeDecal                 // tile mode
      ) * blend_info.dst_input_alpha;

  f16vec4 dst = IPUnpremultiply(dst_sample);
  f16vec4 src = blend_info.color_factor > 0.
                 ? blend_info.color
                 : IPUnpremultiply(IPSampleWithTileMode(
                       texture_sampler_src,           // sampler
                       v_src_texture_coords,          // texture coordinates
                       blend_info.src_y_coord_scale,  // y coordinate scale
                       kTileModeDecal                 // tile mode
                       ));

  f16vec4 blended = f16vec4(Blend(dst.rgb, src.rgb), 1.0hf) * dst.a;

  frag_color = mix(dst_sample, blended, src.a);
}
