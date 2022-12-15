// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/blending.glsl>

vec4 Blend(vec4 dst, vec4 src) {
  return IPBlendModulate(dst, src);
}

#include "atlas_blend.glsl"
