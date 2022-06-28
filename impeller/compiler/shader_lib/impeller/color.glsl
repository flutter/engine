// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COLOR_GLSL_
#define COLOR_GLSL_

#include <impeller/branching.glsl>

vec4 Unpremultiply(vec4 color) {
  return vec4(color.rgb / color.a * GreaterThan(color.a, 0), color.a);
}

#endif
