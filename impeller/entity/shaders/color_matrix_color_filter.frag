// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A color filter that transforms colors through a 4x5 color matrix.
//
// This filter can be used to change the saturation of pixels, convert from YUV to RGB, etc.
//
// 4x5 matrix for transforming the color and alpha components of a Bitmap.
// The matrix can be passed as single array, and is treated as follows:
//
//   [ a, b, c, d, e,
//     f, g, h, i, j,
//     k, l, m, n, o,
//     p, q, r, s, t ]
//
// When applied to a color [R, G, B, A], the resulting color is computed as:
//
//    R’ = a*R + b*G + c*B + d*A + e;
//    G’ = f*R + g*G + h*B + i*A + j;
//    B’ = k*R + l*G + m*B + n*A + o;
//    A’ = p*R + q*G + r*B + s*A + t;
//
// That resulting color [R’, G’, B’, A’] then has each channel clamped to the 0 to 255 range.

uniform FragInfo {
  mat4 m;
  vec4 v;

} frag_info;

uniform sampler2D texture_sampler;

in vec4 v_color;

out vec4 frag_color;

vec4 Unpremultiply(vec4 color) {
  if (color.a == 0) {
    return vec4(0);
  }
  return vec4(color.rgb / color.a, color.a);
}

void main() {
  // unpremultiply first, as filter inputs are premultiplied.
  vec4 color = Unpremultiply(v_color);

  color = frag_info.m * v_color + frag_info.v;

  // we likely need to clamp `frag_color` at this point.
  // color = clamp(color);

  // premultiply the outputs
  frag_color = vec4(color.rgb * color.a, color.a);
}
