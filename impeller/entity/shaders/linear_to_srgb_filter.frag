// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/color.glsl>

// A color filter that applies the sRGB gamma curve to the color.
//
// This filter is used so that the colors are suitable for display in monitors.

uniform sampler2D input_texture;

in vec2 v_position;
out vec4 frag_color;

void main() {
  vec4 input_color = texture(input_texture, v_position);

  // unpremultiply first, as filter inputs are premultiplied.
  // vec4 color = IPUnpremultiply(input_color);

  //bool negative_input = false;

  //if (input_color < 0) {
  //  input_color = -input_color;
  //  negative_input = true;
  //}

  // Apply the sRGB gamma curve to the color as described in
  // https://www.mathworks.com/help/images/ref/lin2rgb.html.

  //color = frag_info.color_m * color + frag_info.color_v;
  //if (input_color >= 0.0031308) {
  //  input_color = 1.055 * pow(input_color, 0.4166) - 0.055;
  //} else {
  //  input_color = 12.92 * input_color;
  //}


  //if (negative_input) {
  //  input_color = -input_color;
  //}
  
  frag_color = input_color;
}
