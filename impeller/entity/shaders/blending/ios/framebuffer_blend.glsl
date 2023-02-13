#version 450

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/blending.glsl>
#include <impeller/color.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

#ifdef IMPELLER_TARGET_METAL_IOS
layout(set = 0,
       binding = 0,
       input_attachment_index = 0) uniform subpassInput uSub;

vec4 ReadDestination() {
  return subpassLoad(uSub);
}
#else

vec4 ReadDestination() {
  return vec4(0);
}
#endif

uniform BlendInfo {
  float dst_input_alpha;
  float src_y_coord_scale;
  float color_factor;
  vec4 color;  // This color input is expected to be unpremultiplied.
}
blend_info;

uniform sampler2D texture_sampler_src;

in vec2 v_src_texture_coords;

out vec4 frag_color;

void main() {
  vec4 dst_sample = ReadDestination();
  if (dst_sample == vec4(0.0)) {
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
  } else {
  vec4 dst = IPUnpremultiply(dst_sample);
  vec4 src = blend_info.color_factor > 0
                 ? blend_info.color
                 : IPUnpremultiply(IPSampleWithTileMode(
                       texture_sampler_src,           // sampler
                       v_src_texture_coords,          // texture coordinates
                       blend_info.src_y_coord_scale,  // y coordinate scale
                       kTileModeDecal                 // tile mode
                       ));

  vec4 blended = vec4(Blend(dst.rgb, src.rgb), 1) * dst.a;

  frag_color = mix(dst_sample, blended, src.a);
  }
}
