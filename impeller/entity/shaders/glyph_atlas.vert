// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>

#include <impeller/transform.glsl>

uniform FrameInfo {
  mat4 mvp;
}
frame_info;

in f16vec2 unit_position;
in f16vec2 destination_position;
in f16vec2 destination_size;
in f16vec2 source_position;
in f16vec2 source_glyph_size;
in float16_t has_color;

out f16vec2 v_unit_position;
out f16vec2 v_source_position;
out f16vec2 v_source_glyph_size;
out float16_t v_has_color;

void main() {
  gl_Position = IPPositionForGlyphPosition(
      frame_info.mvp, unit_position, destination_position, destination_size);
  v_unit_position = unit_position;
  v_source_position = source_position;
  v_source_glyph_size = source_glyph_size;
  v_has_color = has_color;
}