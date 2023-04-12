// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/blending.glsl>
#include <impeller/color.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

uniform f16sampler2D texture_sampler_dst;

// This must be kept in sync with the values in impeller/geometry/color.h
// clear, src, dst are excluded as these do not require blending.

const float16_t kSourceOver = 3.0hf;
const float16_t kDestinationOver = 4.0hf;
const float16_t kSourceIn = 5.0hf;
const float16_t kDestinationIn = 6.0hf;
const float16_t kSourceOut = 7.0hf;
const float16_t kDestinationOut = 8.0hf;
const float16_t kSourceATop = 9.0hf;
const float16_t kDestinationATop = 10.0hf;
const float16_t kXor = 11.0hf;
const float16_t kPlus = 12.0hf;
const float16_t kModulate = 13.0hf;

uniform FragInfo {
  float16_t operation;
  float16_t input_alpha;
  f16vec4 color;
}
frag_info;

in vec2 v_texture_coords;

out f16vec4 frag_color;

f16vec4 Sample(f16sampler2D texture_sampler, vec2 texture_coords) {
// gles 2.0 is the only backend without native decal support.
#ifdef IMPELLER_TARGET_OPENGLES
  return IPSampleDecal(texture_sampler, texture_coords);
#else
  return texture(texture_sampler, texture_coords);
#endif
}

// Note: this shader reduces the number of branches required by conditionally
// modifying the foreground color.
void main() {
  f16vec4 dst_color =
      texture(texture_sampler_dst, v_texture_coords) * frag_info.input_alpha;

  if (frag_info.operation == kSourceOver) {
    frag_color = IPBlendSourceOver(frag_info.color, dst_color);
  } else if (frag_info.operation == kDestinationOver) {
    frag_color = IPBlendDestinationOver(frag_info.color, dst_color);
  } else if (frag_info.operation == kSourceIn) {
    frag_color = IPBlendSourceIn(frag_info.color, dst_color);
  } else if (frag_info.operation == kDestinationIn) {
    frag_color = IPBlendDestinationIn(frag_info.color, dst_color);
  } else if (frag_info.operation == kSourceOut) {
    frag_color = IPBlendSourceOut(frag_info.color, dst_color);
  } else if (frag_info.operation == kDestinationOut) {
    frag_color = IPBlendDestinationOut(frag_info.color, dst_color);
  } else if (frag_info.operation == kSourceATop) {
    frag_color = IPBlendSourceATop(frag_info.color, dst_color);
  } else if (frag_info.operation == kDestinationATop) {
    frag_color = IPBlendDestinationATop(frag_info.color, dst_color);
  } else if (frag_info.operation == kXor) {
    frag_color = IPBlendXOR(frag_info.color, dst_color);
  } else if (frag_info.operation == kPlus) {
    frag_color = IPBlendPlus(frag_info.color, dst_color);
  } else {
    frag_color = IPBlendModulate(frag_info.color, dst_color);
  }
}
