// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GAUSSIAN_HIGHP_GLSL_
#define GAUSSIAN_HIGHP_GLSL_

/// Gaussian distribution function.
float IPGaussian(float x, float sigma) {
  // sqrt(2 * pi)
  const float kSqrtTwoPi = 2.50662827463;
  float variance = sigma * sigma;
  return exp(-0.5f * x * x / variance) / (kSqrtTwoPi * sigma);
}

/// Simpler (but less accurate) approximation of the Gaussian integral.
vec2 IPVec2FastGaussianIntegral(vec2 x, float sigma) {
  // sqrt(3)
  const float kSqrtThree = 1.73205080757;
  return 1.0 / (1.0 + exp(-kSqrtThree / sigma * x));
}

#endif
