// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_angle.h"

namespace flutter {

// This method could just return the results of the math library's
// sin and cos functions but, due to IEEE floating point mantissa
// scaling, the ability to represent numbers close to "1.0" and "-1.0" is
// much more sparse than the ability to represent numbers close to "0.0".
// As a result, this method will check for cases (which happen at the
// quadrant angles of 0,90,180,270 degrees) where one of the pair is
// returning its best answer of 1 or -1 and force the other value to 0.
// This will help avoid "IEEE bit dirt" in transform matrices at quadrant
// angles when trying to represent those angles as irrational numbers
// in a limited-precision floating point world.
DlFVector DlAngle::CosSin() const {
  if (!DlScalar_IsFinite(radians_)) {
    return {1.0f, 0.0f};
  }
  DlScalar c = cosf(radians_);
  DlScalar s;
  if (c == -1.0f || c == 1.0f) {
    s = 0.0f;
  } else {
    s = sinf(radians_);
    if (s == -1.0f || s == 1.0f) {
      c = 0.0f;
    }
  }
  return {c, s};
}

}  // namespace flutter
