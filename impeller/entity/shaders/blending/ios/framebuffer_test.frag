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


out vec4 frag_color;

void main() {
  vec4 dst_sample = ReadDestination();
  if (dst_sample == vec4(0.0)) {
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
  } else {
    frag_color = mix(dst_sample, vec4(0.0, 1.0, 0.0, 1.0), 0.5);
  }
}
