// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>

uniform sampler2D texture_sampler;

uniform GradientInfo {
  float16_t bias;
}
gradient_info;

in vec2 v_position;

out vec4 frag_color;

void main() {
  frag_color = gradient_info.bias * vec4(1.0, 0.0, 0.0, 1.0);
}
