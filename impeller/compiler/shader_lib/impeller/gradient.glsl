// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GRADIENT_GLSL_
#define GRADIENT_GLSL_

#include <impeller/texture.glsl>

#define FIXED_GRADIENT_SIZE 16

/// Compute a gradient color from a buffer of up to 16 colors.
///
/// The provided value for `t` will be processed according to the `tile_mode`.
/// If the `tile_mode` is decal and t is less than 0 or greater than 1, vec4(0)
/// will be returned.
vec4 IPComputeFixedGradient(float t, vec4[FIXED_GRADIENT_SIZE] colors, float colors_length, float tile_mode) {
  if ((t < 0.0 || t > 1.0) && tile_mode == kTileModeDecal) {
    return vec4(0);
  }

  t = IPFloatTile(t, tile_mode);
  if (colors_length == 2) {
    return mix(colors[0], colors[1], t);
  }

  float rough_index =  colors_length * t;
  float lower_index = floor(rough_index);
  int upper_index = int(ceil(rough_index));
  float scale = rough_index - lower_index;

  return mix(colors[int(lower_index)], colors[upper_index], scale);
}

#endif
