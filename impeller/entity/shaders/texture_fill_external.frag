// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision mediump float;

#include <impeller/constants.glsl>
#include <impeller/types.glsl>

uniform sampler2D SAMPLER_EXTERNAL_OES_texture_sampler;

in vec2 v_texture_coords;
IMPELLER_MAYBE_FLAT in float16_t v_alpha;

out vec4 frag_color;

void main() {
  vec4 sampled =
      texture(SAMPLER_EXTERNAL_OES_texture_sampler, v_texture_coords);
  frag_color = sampled * v_alpha;
}
