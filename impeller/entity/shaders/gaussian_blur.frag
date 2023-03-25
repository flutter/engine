// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/texture.glsl>
#include <impeller/types.glsl>

f16vec4 Sample(f16sampler2D tex, f16vec2 coords) {
  return texture(tex, coords);
}

#include "gaussian_blur.glsl"
