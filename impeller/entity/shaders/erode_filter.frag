// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/constants.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

uniform sampler2D texture_sampler;

uniform FragInfo {
  mediump vec2 uv_offset;
  mediump float radius;
}
frag_info;

in vec2 v_texture_coords;
out vec4 frag_color;

void main() {
  vec4 result = vec4(1);
  for (float i = -frag_info.radius; i <= frag_info.radius; i++) {
    vec2 texture_coords = v_texture_coords + frag_info.uv_offset * i;
    vec4 color = IPSampleDecal(texture_sampler,  // sampler
                               texture_coords    // texture coordinates
    );
    result = min(color, result);
  }

  frag_color = result;
}
