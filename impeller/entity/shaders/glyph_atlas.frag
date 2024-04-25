// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision mediump float;

#include <impeller/types.glsl>

uniform f16sampler2D glyph_atlas_sampler;

layout(constant_id = 0) const float use_alpha_color_channel = 1.0;

uniform FragInfo {
  float is_color_glyph;
  float use_text_color;
  f16vec4 text_color;
}
frag_info;

in highp vec2 v_uv;

out vec4 frag_color;

void main() {
  f16vec4 value = texture(glyph_atlas_sampler, v_uv);

  frag_color =
      mix(mix(value.rrrr * frag_info.text_color,
              value.aaaa * frag_info.text_color, use_alpha_color_channel),
          mix(value * frag_info.text_color.aaaa,
              value.aaaa * frag_info.text_color, frag_info.use_text_color),
          frag_info.is_color_glyph);
}
