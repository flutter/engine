// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "advanced_blend_utils.glsl"

vec3 Blend(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingcolorburn
  vec3 color = 1 - min(vec3(1), (1 - dst) / src);
  color = mix(color, vec3(1), ComponentIsValue(dst, 1.0));
  color = mix(color, vec3(0), ComponentIsValue(src, 0.0));
  return color;
}

#include "advanced_blend.glsl"
