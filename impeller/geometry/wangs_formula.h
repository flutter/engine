// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_GEOMETRY_WANGS_FORMULA_H_
#define FLUTTER_IMPELLER_GEOMETRY_WANGS_FORMULA_H_

#include "impeller/geometry/path_component.h"
#include "impeller/geometry/point.h"
#include "impeller/geometry/scalar.h"

// Skia GPU Ports

// Wang's formula gives the minimum number of evenly spaced (in the parametric
// sense) line segments that a bezier curve must be chopped into in order to
// guarantee all lines stay within a distance of "1/precision" pixels from the
// true curve. Its definition for a bezier curve of degree "n" is as follows:
//
//     maxLength = max([length(p[i+2] - 2p[i+1] + p[i]) for (0 <= i <= n-2)])
//     numParametricSegments = sqrt(maxLength * precision * n*(n - 1)/8)
//
// (Goldman, Ron. (2003). 5.6.3 Wang's Formula. "Pyramid Algorithms: A Dynamic
// Programming Approach to Curves and Surfaces for Geometric Modeling". Morgan
// Kaufmann Publishers.)
namespace impeller {

// Don't allow linearized segments to be off by more than 1/4th of a pixel from
// the true curve.
constexpr static Scalar kPrecision = 4;

inline static float length(Point n) {
  Point nn = n * n;
  return std::sqrt(nn.x + nn.y);
}

inline static Point Max(Point a, Point b) {
  return Point{
      a.x > b.x ? a.x : b.x,  //
      a.y > b.y ? a.y : b.y   //
  };
}

inline static float cubic(float intolerance,
                          Point p0,
                          Point p1,
                          Point p2,
                          Point p3) {
  float k = intolerance * .75f * kPrecision;
  Point a = (p0 - p1 * 2 + p2).Abs();
  Point b = (p1 - p2 * 2 + p3).Abs();
  return std::sqrt(k * length(Max(a, b)));
}

inline static float quadratic(float intolerance, Point p0, Point p1, Point p2) {
  float k = intolerance * .25f * kPrecision;
  return std::sqrt(k * length(p0 - p1 * 2 + p2));
}

// Returns the minimum number of evenly spaced (in the parametric sense) line
// segments that the quadratic must be chopped into in order to guarantee all
// lines stay within a distance of "1/intolerance" pixels from the true curve.
inline static float quadratic(float intolerance,
                              const QuadraticPathComponent& quad) {
  return quadratic(intolerance, quad.p1, quad.cp, quad.p2);
}

// Returns the minimum number of evenly spaced (in the parametric sense) line
// segments that the cubic must be chopped into in order to guarantee all lines
// stay within a distance of "1/intolerance" pixels from the true curve.
inline static float cubic(float intolerance, const CubicPathComponent& cub) {
  return cubic(intolerance, cub.p1, cub.cp1, cub.cp2, cub.p2);
}

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_GEOMETRY_WANGS_FORMULA_H_
