// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GRADIENT_GLSL_
#define GRADIENT_GLSL_

#include <impeller/texture.glsl>

/// Compute the indexes and mix coefficient used to mix colors for an
/// arbitrarily sized color gradient.
///
/// The returned values are the lower index, upper index, and mix
/// coefficient.
vec3 IPComputeFixedGradientValues(float t, float colors_length) {
  if (colors_length == 2) {
    return vec3(0, 1, t);
  }

  float rough_index =  colors_length * t;
  float lower_index = floor(rough_index);
  float upper_index = ceil(rough_index);
  float scale = rough_index - lower_index;

  return vec3(lower_index, upper_index, scale);
}

#endif
