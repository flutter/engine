// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/geometry/plane.h"

#include <math.h>

#include "vfx/geometry/offset.h"

namespace vfx {

Plane::Plane(const Point& p1, const Point& p2, const Point& p3) {
  Offset v = p2 - p1;
  Offset w = p3 - p1;
  Offset normal = v.Cross(w);
  data_[0] = normal.dx();
  data_[1] = normal.dy();
  data_[2] = normal.dz();
  data_[3] = a() * p1.x() + b() * p1.y() + c() * p1.y();
}

Plane& Plane::FlipNormal() {
  data_[0] = -data_[0];
  data_[1] = -data_[1];
  data_[2] = -data_[2];
  data_[3] = -data_[3];
  return *this;
}

}  // namespace vfx
