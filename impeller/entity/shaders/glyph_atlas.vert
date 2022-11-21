// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/transform.glsl>
#include <impeller/types.glsl>

uniform FrameInfo {
  mat4 mvp;
}
frame_info;

<<<<<<< HEAD
in f16vec2 unit_vertex;
in f16vec2 glyph_position;
in f16vec2 glyph_size;
in f16vec2 atlas_position;
in f16vec2 atlas_glyph_size;
in float16_t color_glyph;

out f16vec2 v_unit_vertex;
out f16vec2 v_atlas_position;
out f16vec2 v_atlas_glyph_size;
out float16_t v_color_glyph;
=======
in vec2 unit_position;
in vec2 destination_position;
in vec2 destination_size;
in vec2 source_position;
in vec2 source_glyph_size;
in float has_color;

out vec2 v_unit_position;
out vec2 v_source_position;
out vec2 v_source_glyph_size;
out float v_has_color;
>>>>>>> ddf6a20b86578f147ee7da023f3f08ecb4256d07

void main() {
  gl_Position = IPPositionForGlyphPosition(
      frame_info.mvp, unit_position, destination_position, destination_size);
  v_unit_position = unit_position;
  v_source_position = source_position;
  v_source_glyph_size = source_glyph_size;
  v_has_color = has_color;
}
