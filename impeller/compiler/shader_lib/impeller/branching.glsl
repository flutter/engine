// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BRANCHING_GLSL_
#define BRANCHING_GLSL_

#include <impeller/constants.glsl>

vec3 EqualTo3(vec3 x, float y) {
  return 1 - abs(sign(x - y));
}

float GreaterThan(float x, float y) {
  return max(sign(x - y), 0);
}

vec3 GreaterThan3(vec3 x, float y) {
  return max(sign(x - y), 0);
}

float LessThan(float x, float y) {
  return max(sign(y - x), 0);
}

vec3 MixCutoff(vec3 a, vec3 b, vec3 value, float cutoff) {
  return mix(a, b, GreaterThan3(value, cutoff));
}

vec3 MixHalf(vec3 a, vec3 b, vec3 value) {
  return MixCutoff(a, b, value, 0.5);
}

#endif
