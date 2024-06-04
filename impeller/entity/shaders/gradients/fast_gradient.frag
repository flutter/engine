// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision mediump float;

#include <impeller/color.glsl>
#include <impeller/dithering.glsl>
#include <impeller/types.glsl>

uniform FragInfo {
  float alpha;
}
frag_info;

uniform sampler2D dither_lut;

in vec4 v_color;

out vec4 frag_color;

void main() {
  frag_color = IPPremultiply(v_color) * frag_info.alpha;
  IPOrderedDither8x8(frag_color, gl_FragCoord.xy, dither_lut);
}
