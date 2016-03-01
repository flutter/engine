// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/geometry/wedge.h"

namespace vfx {

Plane Wedge::GetPlane(size_t index) const {
  switch (index) {
  case 0:
    return a_.GetPlane();;
  case 1:
    return b_.GetPlane();;
  case 2:
    return Plane(a_[0], a_[2], b_[2]);
  case 3:
    return Plane(a_[1], a_[0], b_[0]);
  case 4:
    return Plane(a_[2], a_[1], b_[1]);
  }
  return Plane();
}

}  // namespace vfx
