// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision mediump float;

#include <impeller/color.glsl>
#include <impeller/dithering.glsl>
#include <impeller/texture.glsl>
#include <impeller/types.glsl>

struct ColorPoint {
  vec4 color;
  float stop;
  float inverse_delta;
};

layout(std140) readonly buffer ColorData {
  ColorPoint colors[];
}
color_data;

uniform FragInfo {
  highp vec4 packed_points;
  // alpha, tile_mode, colors_length, inverse_dot_start_to_end
  vec4 packed_components;
  vec4 decal_border_color;
}
frag_info;

highp in vec2 v_position;

out vec4 frag_color;

void main() {
  highp vec2 start_point = frag_info.packed_points.xy;
  highp vec2 start_to_end = frag_info.packed_points.zw;
  float alpha = frag_info.packed_components.x;
  float tile_mode = frag_info.packed_components.y;
  int colors_length = int(frag_info.packed_components.z);
  float inverse_dot_start_to_end = frag_info.packed_components.w;

  highp vec2 start_to_position = v_position - start_point;
  highp float t =
      dot(start_to_position, start_to_end) * inverse_dot_start_to_end;

  if ((t < 0.0 || t > 1.0) && tile_mode == kTileModeDecal) {
    frag_color = frag_info.decal_border_color;
  } else {
    t = IPFloatTile(t, tile_mode);

    for (int i = 1; i < colors_length; i++) {
      ColorPoint prev_point = color_data.colors[i - 1];
      ColorPoint current_point = color_data.colors[i];
      if (t >= prev_point.stop && t <= current_point.stop) {
        if (current_point.inverse_delta > 1000.0) {
          frag_color = current_point.color;
        } else {
          float ratio = (t - prev_point.stop) * current_point.inverse_delta;
          frag_color = mix(prev_point.color, current_point.color, ratio);
        }
        break;
      }
    }
  }

  frag_color = IPPremultiply(frag_color) * alpha;
  frag_color = IPOrderedDither8x8(frag_color, gl_FragCoord.xy);
}
