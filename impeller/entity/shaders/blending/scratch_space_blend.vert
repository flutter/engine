// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/conversions.glsl>
#include <impeller/types.glsl>

// Warning: if any of the constant values or layouts are changed in this
// file, then the hard-coded constant value in
// impeller/renderer/backend/vulkan/binding_helpers_vk.cc
uniform FrameInfo {
  mat4 mvp;
}
frame_info;

in vec2 vertices;

void main() {
  gl_Position = frame_info.mvp * vec4(vertices, 0.0, 1.0);
}
