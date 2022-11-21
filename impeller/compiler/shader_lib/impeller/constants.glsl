// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONSTANTS_GLSL_
#define CONSTANTS_GLSL_

#include <impeller/types.glsl>

const float16_t kEhCloseEnough = 0.000001hf;

// 1 / (2 * pi)
const float16_t k1Over2Pi = 0.1591549430918hf;

// sqrt(2 * pi)
const float16_t kSqrtTwoPi = 2.50662827463hf;

// sqrt(2) / 2 == 1 / sqrt(2)
const float16_t kHalfSqrtTwo = 0.70710678118hf;

#endif
