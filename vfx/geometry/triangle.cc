// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/geometry/triangle.h"

#include <utility>

namespace vfx {

Plane Triangle::GetPlane() const {
  return Plane(p1(), p2(), p3());
}

Triangle& Triangle::FlipNormal() {
  std::swap(data_[1], data_[2]);
  return *this;
}

}  // namespace vfx
