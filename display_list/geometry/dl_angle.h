// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ANGLE_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ANGLE_H_

#include <algorithm>
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
/// APIs that take an angle as a parameter should use |DlAngle| to specify
/// their arguments.
///
/// Developers specifying an angle to an API need only construct a
/// |DlDegrees| or |DlRadians| object as their code finds most convenient
/// without regard to how the angle will eventually be used.
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
  /// Returns the angle measured in radians, converting if needed.
  virtual DlScalar radians() const = 0;

  /// Returns the angle measured in degrees, converting if needed.
  virtual DlScalar degrees() const = 0;

  /// Returns a vector containing the cosine of the angle in the
  /// x value and the sine of the angle in the y value. The method
  /// will attempt to adjust for some common practical cases where
  /// some angles produce values such that |cos^2 + sin^2 > 1.0|
  DlFVector CosSin() const;
};

/// An angle constructed from a value in degrees which can be handed to
/// any API that takes a |DlAngle|. The value will automatically be
/// converted to radians if the API being called requires them.
class DlDegrees : public DlAngle {
 public:
  constexpr DlDegrees() : DlDegrees(0.0) {}
  constexpr explicit DlDegrees(DlScalar degrees) : degrees_(degrees) {}
  constexpr DlDegrees(const DlAngle& angle) : degrees_(angle.degrees()) {}

  DlScalar radians() const override {
    return static_cast<DlScalar>(degrees_ * kDlScalar_Pi / 180.0);
  }
  DlScalar degrees() const override { return degrees_; }

 private:
  DlScalar degrees_;
};

/// An angle constructed from a value in degrees which can be handed to
/// any API that takes a |DlAngle|. The value will automatically be
/// converted to radians if the API being called requires them.
class DlRadians : public DlAngle {
 public:
  constexpr DlRadians() : DlRadians(0.0) {}
  constexpr explicit DlRadians(DlScalar radians) : radians_(radians) {}
  constexpr DlRadians(const DlAngle& angle) : radians_(angle.radians()) {}

  DlScalar radians() const override { return radians_; }
  DlScalar degrees() const override {
    return static_cast<DlScalar>(radians_ * 180.0 / kDlScalar_Pi);
  }

 private:
  DlScalar radians_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ANGLE_H_
