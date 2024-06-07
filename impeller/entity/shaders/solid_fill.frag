// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision mediump float;

#include <impeller/types.glsl>

uniform FragInfo {
  vec4 color;
  bool use_scratch_space;
}
frag_info;

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 scratch_space;

void main() {
  if (frag_info.use_scratch_space) {
    scratch_space = frag_info.color;
    frag_color = vec4(0);
  } else {
    scratch_space = vec4(0);
    frag_color = frag_info.color;
  }
}
