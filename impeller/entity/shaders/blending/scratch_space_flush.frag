#version 450

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/blending.glsl>
#include <impeller/color.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>
#include "blend_select.glsl"

layout(input_attachment_index = 1) uniform subpassInputMS scratch_space_input;

vec4 ReadScratchSpace() {
  return (subpassLoad(scratch_space_input, 0) + subpassLoad(scratch_space_input, 1) + subpassLoad(scratch_space_input, 2) +
          subpassLoad(scratch_space_input, 3)) /
         vec4(4.0);
}

uniform FragInfo {
  float alpha;
} frag_info;

uniform sampler2D texture_sampler_src;


in vec2 v_src_texture_coords;

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 scratch_space;

void main() {
  frag_color = ReadScratchSpace() * frag_info.alpha;
  scratch_space = vec4(0);
}
