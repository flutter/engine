// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>

uniform sampler2D glyph_atlas_sampler;

uniform FragInfo {
  f16vec2 atlas_size;
  f16vec4 text_color;
}
frag_info;

in f16vec2 v_unit_vertex;
in f16vec2 v_atlas_position;
in f16vec2 v_atlas_glyph_size;
in float16_t v_color_glyph;

out f16vec4 frag_color;

void main() {
  f16vec2 scale_perspective = v_atlas_glyph_size / frag_info.atlas_size;
  f16vec2 offset = v_atlas_position / frag_info.atlas_size;
  if (v_color_glyph == 1.0hf) {
    frag_color = f16vec4(texture(glyph_atlas_sampler,
                                 v_unit_vertex * scale_perspective + offset));
  } else {
    frag_color = f16vec4(texture(glyph_atlas_sampler,
                                 v_unit_vertex * scale_perspective + offset)
                             .aaaa) *
                 frag_info.text_color;
  }
}
