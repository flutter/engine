// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/timing_curve.h"

namespace impeller::testing {

static inline Scalar TimingCurve_SampleCurve(Scalar a,
                                             Scalar b,
                                             Scalar c,
                                             Scalar t) {
  /*
   *  `a t^3 + b t^2 + c t' expanded using Horner's rule.
   */
  return ((a * t + b) * t + c) * t;
}

static inline Scalar TimingCurve_SampleCurveDerivative(Scalar a,
                                                       Scalar b,
                                                       Scalar c,
                                                       Scalar t) {
  return (3.0 * a * t + 2.0 * b) * t + c;
}

static inline Scalar TimingCurve_SolveCurveX(Scalar ax,
                                             Scalar bx,
                                             Scalar cx,
                                             Scalar x,
                                             Scalar epsilon) {
  Scalar t0 = 0.0;
  Scalar t1 = 0.0;
  Scalar t2 = 0.0;
  Scalar x2 = 0.0;
  Scalar d2 = 0.0;
  int i = 0;

  /*
   *  Try Newton's method
   */
  for (t2 = x, i = 0; i < 8; i++) {
    x2 = TimingCurve_SampleCurve(ax, bx, cx, t2) - x;

    if (fabs(x2) < epsilon) {
      return t2;
    }

    d2 = TimingCurve_SampleCurveDerivative(ax, bx, cx, t2);

    if (fabs(d2) < epsilon) {
      break;
    }

    t2 = t2 - x2 / d2;
  }

  /*
   *  Fall back to bisection
   */
  t0 = 0.0;
  t1 = 1.0;
  t2 = x;

  if (t2 < t0) {
    return t0;
  }

  if (t2 > t1) {
    return t1;
  }

  while (t0 < t1) {
    x2 = TimingCurve_SampleCurve(ax, bx, cx, t2);

    if (fabs(x2 - x) < epsilon)
      return t2;

    if (x > x2) {
      t0 = t2;
    } else {
      t1 = t2;
    }

    t2 = (t1 - t0) * 0.5 + t0;
  }

  /*
   *  Failure
   */
  return t2;
}

static inline Scalar TimingCurve_SolveX(Scalar ax,
                                        Scalar bx,
                                        Scalar cx,
                                        Scalar ay,
                                        Scalar by,
                                        Scalar cy,
                                        Scalar x,
                                        Scalar epsilon) {
  const Scalar xSolution = TimingCurve_SolveCurveX(ax, bx, cx, x, epsilon);
  return TimingCurve_SampleCurve(ay, by, cy, xSolution);
}

TimingCurve TimingCurve::SystemTimingCurve(Type type) {
  switch (type) {
    case Type::kLinear:
      return TimingCurve({0.0, 0.0}, {1.0, 1.0});
    case Type::kEaseIn:
      return TimingCurve({0.42, 0.0}, {1.0, 1.0});
    case Type::kEaseOut:
      return TimingCurve({0.0, 0.0}, {0.58, 1.0});
    case Type::kEaseInEaseOut:
      return TimingCurve({0.42, 0.0}, {0.58, 1.0});
  }

  return TimingCurve({0.0, 0.0}, {1.0, 1.0});
}

TimingCurve::TimingCurve(const Point& c1, const Point& c2) {
  cx_ = 3.0 * c1.x;
  bx_ = 3.0 * (c2.x - c1.x) - cx_;
  ax_ = 1.0 - cx_ - bx_;

  cy_ = 3.0 * c1.y;
  by_ = 3.0 * (c2.y - c1.y) - cy_;
  ay_ = 1.0 - cy_ - by_;
}

Scalar TimingCurve::x(Scalar t) const {
  return TimingCurve_SolveX(ax_, bx_, cx_, ay_, by_, cy_, t, 1e-3);
}

}  // namespace impeller::testing
