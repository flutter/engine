// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/geometry/point.h"

#include <math.h>

namespace vfx {

float Offset::NormSquared() const {
  return data_[0] * data_[0] + data_[1] * data_[1] + data_[2] * data_[2];
}

Offset& Offset::Normalize() {
  float norm = sqrt(NormSquared());
  data_[0] /= norm;
  data_[1] /= norm;
  data_[2] /= norm;
  return *this;
}

Offset Offset::Cross(const Offset& v) {
  return Offset(dy() * v.dz() - dz() * v.dy(),
                dz() * v.dx() - dx() * v.dz(),
                dx() * v.dy() - dy() * v.dx());
}

}  // namespace vfx
