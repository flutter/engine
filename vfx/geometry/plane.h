// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GEOMETRY_PLANE_H_
#define VFX_GEOMETRY_PLANE_H_

#include <memory>

#include "vfx/geometry/point.h"

namespace vfx {

// A plane in three dimensions defined by ax + by + cz = d.
class Plane {
 public:
  Plane() { memset(data_, 0, sizeof(data_)); }
  explicit Plane(float data[4]) { memcpy(data_, data, sizeof(data_)); }
  Plane(float a, float b, float c, float d) {
    data_[0] = a;
    data_[1] = b;
    data_[2] = c;
    data_[3] = d;
  }
  Plane(const Point& p1, const Point& p2, const Point& p3);

  float a() const { return data_[0]; }
  float b() const { return data_[1]; }
  float c() const { return data_[2]; }
  float d() const { return data_[3]; }

  const float* data() const { return data_; }

  void set_a(float a) { data_[0] = a; }
  void set_b(float b) { data_[1] = b; }
  void set_c(float c) { data_[2] = c; }
  void set_d(float d) { data_[3] = d; }

  Plane& FlipNormal();

 private:
  float data_[4];
};

}  // namespace vfx

#endif  // VFX_GEOMETRY_PLANE_H_
