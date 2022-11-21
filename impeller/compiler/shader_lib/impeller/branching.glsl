// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BRANCHING_GLSL_
#define BRANCHING_GLSL_

#include <impeller/types.glsl>
#include <impeller/constants.glsl>

/// Perform an equality check for each vec3 component.
///
/// Returns 1.0 if x == y, otherwise 0.0.
BoolV3 IPVec3IsEqual(f16vec3 x, float16_t y) {
  f16vec3 diff = abs(x - y);
  return f16vec3(diff.r < float16_t(kEhCloseEnough),  //
              diff.g < float16_t(kEhCloseEnough),  //
              diff.b < float16_t(kEhCloseEnough));
}

/// Perform a branchless greater than check.
///
/// Returns 1.0 if x > y, otherwise 0.0.
BoolF IPFloatIsGreaterThan(float16_t x, float16_t y) {
  return max(sign(x - y), 0.0hf);
}

/// Perform a branchless greater than check for each vec3 component.
///
/// Returns 1.0 if x > y, otherwise 0.0.
BoolV3 IPVec3IsGreaterThan(f16vec3 x, f16vec3 y) {
  return max(sign(x - y), 0.0hf);
}

/// Perform a branchless less than check.
///
/// Returns 1.0 if x < y, otherwise 0.0.
BoolF IPFloatIsLessThan(float16_t x, float16_t y) {
  return max(sign(y - x), 0.0hf);
}

/// For each vec3 component, if value > cutoff, return b, otherwise return a.
f16vec3 IPVec3ChooseCutoff(f16vec3 a, f16vec3 b, f16vec3 value, float16_t cutoff) {
  return mix(a, b, IPVec3IsGreaterThan(value, f16vec3(cutoff)));
}

/// For each vec3 component, if value > 0.5, return b, otherwise return a.
f16vec3 IPVec3Choose(f16vec3 a, f16vec3 b, f16vec3 value) {
  return IPVec3ChooseCutoff(a, b, value, 0.5hf);
}

#endif
