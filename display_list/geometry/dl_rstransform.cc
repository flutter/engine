// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_rstransform.h"

namespace flutter {

void DlRSTransform::ToQuad(DlScalar width,
                           DlScalar height,
                           DlFVector quad[4]) const {
  // delta_transform(x, y) = {x * cos - y * sin, x * sin + y * cos}
  // h vector = delta_transform(width, 0)
  // v vector = delta_transform(0, height)
  DlFVector h = width * DlFVector(scaled_cos(), scaled_sin());
  DlFVector v = height * DlFVector(-scaled_sin(), scaled_cos());
  // The quad is a parallelogram with its origin at tx,ty and with
  // the sides defined by the delta vectors hx,hy and vx,vy. The
  // two near corners have only one of the delta vectors added,
  // the far corner has both added.
  // Going clockwise starting from the UL (origin) corner:
  quad[0] = origin_;
  quad[1] = quad[0] + h;
  quad[2] = quad[1] + v;
  quad[3] = quad[0] + v;
}

DlAngle DlRSTransform::ExtractAngle() const {
  DlFVector cos_sin = trig_ / trig_.Length();
  return DlAngle::Radians(atan2f(cos_sin.y(), cos_sin.x()));
}

std::ostream& operator<<(std::ostream& os, const DlRSTransform& rst) {
  return os << "DlRSTransform("             //
            << rst.ExtractAngle() << " * "  //
            << rst.ExtractScale() << " @ "  //
            << rst.translate_x() << ", "    //
            << rst.translate_y() << ")";
}

}  // namespace flutter
