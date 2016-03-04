// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/geometry/quad.h"

#include "vfx/geometry/offset.h"

namespace vfx {

void Quad::Move(const Offset& offset) {
  data_[0].Move(offset);
  data_[1].Move(offset);
  data_[2].Move(offset);
  data_[3].Move(offset);
}

Quad Quad::ProjectDistanceFromSource(const Point& source,
                                     double distance) const {
  Offset ray[4] = {
    p1() - source,
    p2() - source,
    p3() - source,
    p4() - source,
  };

  ray[0].Normalize();
  ray[1].Normalize();
  ray[2].Normalize();
  ray[3].Normalize();

  ray[0].Scale(distance);
  ray[1].Scale(distance);
  ray[2].Scale(distance);
  ray[3].Scale(distance);

  return Quad(source + ray[0],
              source + ray[1],
              source + ray[2],
              source + ray[3]);
}

Offset Quad::GetNormal() const {
  Offset v = p2() - p1();
  Offset w = p3() - p1();
  return v.Cross(w);
}

Offset Quad::GetUnitNormal() const {
  Offset normal = GetNormal();
  normal.Normalize();
  return normal;
}

Plane Quad::GetPlane() const {
  return Plane(p1(), p2(), p3());
}

}  // namespace vfx
