#version 450

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/blending.glsl>
#include <impeller/color.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>
#include "blend_select.glsl"

// Warning: if any of the constant values or layouts are changed in this
// file, then the hard-coded constant value in
// impeller/renderer/backend/vulkan/binding_helpers_vk.cc
layout(constant_id = 0) const float blend_type = 0;

layout(input_attachment_index = 0) uniform subpassInputMS destination_input;
layout(input_attachment_index = 1) uniform subpassInputMS scratch_space_input;

vec4 ReadDestination() {
  return (subpassLoad(destination_input, 0) + subpassLoad(destination_input, 1) + subpassLoad(destination_input, 2) +
          subpassLoad(destination_input, 3)) /
         vec4(4.0);
}

vec4 ReadScratchSpace() {
  return (subpassLoad(scratch_space_input, 0) + subpassLoad(scratch_space_input, 1) + subpassLoad(scratch_space_input, 2) +
          subpassLoad(scratch_space_input, 3)) /
         vec4(4.0);
}

uniform sampler2D texture_sampler_src;

uniform FragInfo {
  float16_t src_input_alpha;
}
frag_info;

in vec2 v_src_texture_coords;

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 scratch_space;

void main() {
  f16vec4 dst = IPHalfUnpremultiply(f16vec4(ReadDestination()));
  f16vec4 src = IPHalfUnpremultiply(f16vec4(ReadScratchSpace()));

  f16vec3 blend_result = AdvancedBlend(dst.rgb, src.rgb, int(blend_type));

  frag_color = IPApplyBlendedColor(dst, src, blend_result);
  scratch_space = vec4(0);
}
