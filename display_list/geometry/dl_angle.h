// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ANGLE_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ANGLE_H_

#include <algorithm>
#include <cmath>
#include <limits>
#include <ostream>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/geometry/dl_point.h"

namespace flutter {

/// An angle readable in either radians or degrees, that may have originally
/// be specified in either radians or degrees.
///
/// This holder object removes the need for APIs to specify whether or not
/// they take degrees or radians and for developers to be aware of that
/// aspect of the API, providing format conversions on the fly only when
/// necessary.
///
/// A method |CosSin| is provided to compute both sine and cosine of an
/// angle at the same time and attempts to avoid tricky practical cases
/// where the 2 values produce a vector longer than 1.0 due to differences
/// in precision for values near 1.0 and values near 0.0.
///
/// NaN and Infinity handling
///
/// The |CosSin| method will return the vector {1, 0} as if the angle was
/// 0 degrees/radians if the value being stored is a NaN or +/-Infinity.
class DlAngle {
 public:
  constexpr DlAngle() : radians_(0.0f) {}
  static constexpr DlAngle Radians(DlScalar radians = 0.0f) {
    return DlAngle(radians);
  }
  static constexpr DlAngle Degrees(DlScalar degrees = 0.0f) {
    return DlAngle(degrees * kDlScalar_Pi / 180.0f);
  }

  /// Returns the angle measured in radians, converting if needed.
  constexpr DlScalar radians() const { return radians_; }

  /// Returns the angle measured in degrees, converting if needed.
  constexpr DlScalar degrees() const {
    return radians_ * 180.0f / kDlScalar_Pi;
  }

  /// Returns a vector containing the cosine of the angle in the
  /// x value and the sine of the angle in the y value. The method
  /// will attempt to adjust for some common practical cases where
  /// some angles produce values such that |cos^2 + sin^2 > 1.0|
  DlFVector CosSin() const;

  constexpr bool IsFullCircle() const {
    DlScalar r = (remainder(radians_, kDlScalar_Pi * 2.0f));
    return !(fabs(r) > kDlScalar_NearlyZero);
  }

  constexpr DlAngle operator+(DlAngle other) {
    return DlAngle(this->radians_ + other.radians_);
  }

  constexpr DlAngle operator-(DlAngle other) {
    return DlAngle(this->radians_ - other.radians_);
  }

  constexpr DlAngle operator*(DlScalar s) {
    return DlAngle(this->radians_ * s);
  }

  constexpr DlAngle operator/(DlScalar s) {
    return DlAngle(this->radians_ / s);
  }

 private:
  constexpr explicit DlAngle(DlScalar radians)
      : radians_(DlScalar_IsFinite(radians) ? radians : 0.0f) {}

  const DlScalar radians_;

  friend DlAngle operator*(DlAngle a, DlScalar s);
  friend DlAngle operator*(DlScalar s, DlAngle a);
  friend DlAngle operator/(DlAngle a, DlScalar s);
};

inline std::ostream& operator<<(std::ostream& os, const DlAngle& angle) {
  return os << "DlAngle("                //
            << angle.radians() << "r, "  //
            << angle.degrees() << "d)";
}

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ANGLE_H_
