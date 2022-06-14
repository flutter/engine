// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

uniform FragmentInfo {
  vec4 color;
}
fragment_info;

in vec2 v_border;

out vec4 frag_color;

// Simple logistic sigmoid with a domain and range of 0 to 1.
float Sigmoid(float x) {
  return 1.03597241992 / (1 + exp(-8 * x + 4)) - 0.0179862099621;
}

void main() {
  float shadow_mask = Sigmoid(max(0.0, 1.0 - length(v_border)));
  frag_color = fragment_info.color * shadow_mask;
}
