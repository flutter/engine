// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/geometry/point.h"

#include <math.h>

#include "vfx/geometry/offset.h"

namespace vfx {

void Point::Move(const Offset& offset) {
  data_[0] += offset.dx();
  data_[1] += offset.dy();
  data_[2] += offset.dz();
}

}  // namespace vfx
