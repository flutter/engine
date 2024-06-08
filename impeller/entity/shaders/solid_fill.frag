// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision mediump float;

#include <impeller/types.glsl>

uniform FragInfo {
  vec4 color;
}
frag_info;

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 scratch_space;

void main() {
  scratch_space = frag_info.color;
  frag_color = frag_info.color;
}
