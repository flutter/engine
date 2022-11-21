// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GRADIENT_GLSL_
#define GRADIENT_GLSL_

#include <impeller/types.glsl>
#include <impeller/texture.glsl>

/// Compute the indexes and mix coefficient used to mix colors for an
/// arbitrarily sized color gradient.
///
/// The returned values are the lower index, upper index, and mix
/// coefficient.
f16vec3 IPComputeFixedGradientValues(float16_t t, float16_t colors_length) {
  float16_t rough_index = (colors_length - 1.0hf) * t;
  float16_t lower_index = floor(rough_index);
  float16_t upper_index = ceil(rough_index);
  float16_t scale = rough_index - lower_index;

  return f16vec3(lower_index, upper_index, scale);
}

#endif
