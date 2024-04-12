// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/geometry/wangs_formula.h"

namespace impeller {

namespace {

// Don't allow linearized segments to be off by more than 1/4th of a pixel from
// the true curve.
constexpr static Scalar kPrecision = 4;

static inline Scalar length(Point n) {
  Point nn = n * n;
  return std::sqrt(nn.x + nn.y);
}

static inline Point Max(Point a, Point b) {
  return Point{
      a.x > b.x ? a.x : b.x,  //
      a.y > b.y ? a.y : b.y   //
  };
}

}  // namespace

Scalar ComputeCubicSubdivisions(Scalar intolerance,
                                Point p0,
                                Point p1,
                                Point p2,
                                Point p3) {
  Scalar k = intolerance * .75f * kPrecision;
  Point a = (p0 - p1 * 2 + p2).Abs();
  Point b = (p1 - p2 * 2 + p3).Abs();
  return std::sqrt(k * length(Max(a, b)));
}

Scalar ComputeQuadradicSubdivisions(Scalar intolerance,
                                    Point p0,
                                    Point p1,
                                    Point p2) {
  Scalar k = intolerance * .25f * kPrecision;
  return std::sqrt(k * length(p0 - p1 * 2 + p2));
}

Scalar ComputeQuadradicSubdivisions(Scalar intolerance,
                                    const QuadraticPathComponent& quad) {
  return ComputeQuadradicSubdivisions(intolerance, quad.p1, quad.cp, quad.p2);
}

Scalar ComputeCubicSubdivisions(float intolerance,
                                const CubicPathComponent& cub) {
  return ComputeCubicSubdivisions(intolerance, cub.p1, cub.cp1, cub.cp2,
                                  cub.p2);
}

}  // namespace impeller
