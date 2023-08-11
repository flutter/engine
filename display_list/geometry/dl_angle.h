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

class DlAngle {
 public:
  virtual DlScalar radians() const = 0;
  virtual DlScalar degrees() const = 0;

  DlFVector CosSin() const;
};

class DlDegrees : public DlAngle {
 public:
  constexpr DlDegrees() : DlDegrees(0.0) {}
  constexpr explicit DlDegrees(DlScalar degrees) : degrees_(degrees) {}
  constexpr DlDegrees(const DlAngle& angle) : degrees_(angle.degrees()) {}

  DlScalar radians() const override {
    return static_cast<DlScalar>(degrees_ * M_PI / 180.0);
  }
  DlScalar degrees() const override { return degrees_; }

 private:
  DlScalar degrees_;
};

class DlRadians : public DlAngle {
 public:
  constexpr DlRadians() : DlRadians(0.0) {}
  constexpr explicit DlRadians(DlScalar radians) : radians_(radians) {}
  constexpr DlRadians(const DlAngle& angle) : radians_(angle.radians()) {}

  DlScalar radians() const override { return radians_; }
  DlScalar degrees() const override {
    return static_cast<DlScalar>(radians_ * 180.0 / M_PI);
  }

 private:
  DlScalar radians_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ANGLE_H_
