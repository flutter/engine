// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_DISPLAY_LIST_TIMING_CURVE_H_
#define FLUTTER_IMPELLER_DISPLAY_LIST_TIMING_CURVE_H_

#include "impeller/geometry/point.h"

namespace impeller::testing {

class TimingCurve {
 public:
  enum class Type {
    kLinear,
    kEaseIn,
    kEaseOut,
    kEaseInEaseOut,
  };

  static TimingCurve SystemTimingCurve(Type type);

  Scalar x(Scalar t) const;

 private:
  Scalar ax_ = 0.0;
  Scalar bx_ = 0.0;
  Scalar cx_ = 0.0;
  Scalar ay_ = 0.0;
  Scalar by_ = 0.0;
  Scalar cy_ = 0.0;

  TimingCurve(const Point& c1, const Point& c2);
};

}  // namespace impeller::testing

#endif  // FLUTTER_IMPELLER_DISPLAY_LIST_TIMING_CURVE_H_
