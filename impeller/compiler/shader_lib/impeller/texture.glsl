// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEXTURE_GLSL_
#define TEXTURE_GLSL_

#include <impeller/branching.glsl>

vec4 ImpellerTexture(sampler2D texture_sampler,
                     vec2 coords,
                     float y_coord_scale) {
  if (y_coord_scale < 0.0) {
    coords.y = 1.0 - coords.y;
  }
  return texture(texture_sampler, coords);
}

// Emulate SamplerAddressMode::ClampToBorder.
vec4 SampleWithBorder(sampler2D tex, vec2 uv) {
  float within_bounds = GreaterThan(uv.x, 0) * GreaterThan(uv.y, 0) *
                        LessThan(uv.x, 1) * LessThan(uv.y, 1);
  return texture(tex, uv) * within_bounds;
}

#endif
