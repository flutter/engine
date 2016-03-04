// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GEOMETRY_SPHERE_H_
#define VFX_GEOMETRY_SPHERE_H_

#include "vfx/geometry/point.h"

namespace vfx {

class Sphere {
 public:
  Sphere() : radius_(0.0f) { }
  Sphere(const Point& center, double radius)
    : center_(center), radius_(radius) { }

  const Point& center() const { return center_; }
  double radius() const { return radius_; }

  void set_center(const Point& center) { center_ = center; }
  void set_radius(double radius) { radius_ = radius; }

 private:
  Point center_;
  double radius_;
};

}  // namespace vfx

#endif  // VFX_GEOMETRY_SPHERE_H_
