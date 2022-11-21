// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>

uniform sampler2D glyph_atlas_sampler;

uniform FragInfo {
<<<<<<< HEAD
  f16vec2 atlas_size;
  f16vec4 text_color;
} frag_info;

in f16vec2 v_unit_vertex;
in f16vec2 v_atlas_position;
in f16vec2 v_atlas_glyph_size;
in float16_t v_color_glyph;
=======
  vec2 atlas_size;
  vec4 text_color;
}
frag_info;

in vec2 v_unit_position;
in vec2 v_source_position;
in vec2 v_source_glyph_size;
in float v_has_color;
>>>>>>> ddf6a20b86578f147ee7da023f3f08ecb4256d07

out f16vec4 frag_color;

void main() {
<<<<<<< HEAD
  f16vec2 scale_perspective = v_atlas_glyph_size / frag_info.atlas_size;
  f16vec2 offset = v_atlas_position / frag_info.atlas_size;
  if (v_color_glyph == 1.0hf) {
    frag_color = f16vec4(texture(
      glyph_atlas_sampler,
      v_unit_vertex * scale_perspective + offset
    ));
  } else {
    frag_color = f16vec4(texture(
      glyph_atlas_sampler,
      v_unit_vertex * scale_perspective + offset
    ).aaaa) * frag_info.text_color;
=======
  vec2 uv_size = v_source_glyph_size / frag_info.atlas_size;
  vec2 offset = v_source_position / frag_info.atlas_size;
  if (v_has_color == 1.0) {
    frag_color =
        texture(glyph_atlas_sampler, v_unit_position * uv_size + offset);
  } else {
    frag_color =
        texture(glyph_atlas_sampler, v_unit_position * uv_size + offset).aaaa *
        frag_info.text_color;
>>>>>>> ddf6a20b86578f147ee7da023f3f08ecb4256d07
  }
}
