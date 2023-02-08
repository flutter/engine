// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>
#include <impeller/texture.glsl>

f16vec4 Sample(sampler2D tex, f16vec2 coords) {
  return f16vec4(IPSampleDecal(tex, coords));
}

#include "gaussian_ssbo_blur.glsl"
