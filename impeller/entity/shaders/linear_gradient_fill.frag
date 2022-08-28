// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>

uniform GradientInfo {
  vec4 colors_0;
  vec4 colors_1;
  vec4 colors_2;
  vec4 colors_3;
  vec4 colors_4;
  vec4 colors_5;
  vec4 colors_6;
  vec4 colors_7;

  float stops_0;
  float stops_1;
  float stops_2;
  float stops_3;
  float stops_4;
  float stops_5;
  float stops_6;
  float stops_7;

  vec2 start_point;
  vec2 end_point;
  float last_index;
  float tile_mode;
} gradient_info;

in vec2 v_position;

out vec4 frag_color;

void main() {
  float len = length(gradient_info.end_point - gradient_info.start_point);
  float dot = dot(
    v_position - gradient_info.start_point,
    gradient_info.end_point - gradient_info.start_point
  );
  float t = dot / (len * len);
  if ((t < 0.0 || t > 1.0) && gradient_info.tile_mode == kTileModeDecal) {
    frag_color = vec4(0);
    return;
  }
  t = IPFloatTile(t, gradient_info.tile_mode);
  
  vec4 lower_color;
  vec4 higher_color;
  float lower_stop;
  float higher_stop;
  float stops[8] = {
    gradient_info.stops_0,
    gradient_info.stops_1,
    gradient_info.stops_2,
    gradient_info.stops_3,
    gradient_info.stops_4,
    gradient_info.stops_5,
    gradient_info.stops_6,
    gradient_info.stops_7
  };
  vec4 colors[8] = {
    gradient_info.colors_0,
    gradient_info.colors_1,
    gradient_info.colors_2,
    gradient_info.colors_3,
    gradient_info.colors_4,
    gradient_info.colors_5,
    gradient_info.colors_6,
    gradient_info.colors_7
  };
  for (int i = 0; i < gradient_info.last_index + 1; i++) {
    float stop = stops[i];
    if (stop < t) {
      lower_color = colors[i];
      lower_stop = stop;
    } else if (stop > t) {
      higher_color = colors[i];
      higher_stop = stop;
      break;
    } else {
      lower_color = colors[i];
      higher_color = colors[i];
      lower_stop = stop;
      higher_stop = stop;
      break;
    }
  }

  frag_color = mix(lower_color, higher_color, (t - lower_stop) / (higher_stop - lower_stop));
}
