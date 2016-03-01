// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GEOMETRY_TRIANGLE_H_
#define VFX_GEOMETRY_TRIANGLE_H_

#include <memory>

#include "vfx/geometry/point.h"
#include "vfx/geometry/plane.h"

namespace vfx {

// A region bounded by three lines in a plane.
class Triangle {
 public:
  Triangle() { memset(data_, 0, sizeof(data_)); }
  explicit Triangle(Point data[3]) { memcpy(data_, data, sizeof(data_)); }
  Triangle(const Point& p1, const Point& p2, const Point& p3) {
    data_[0] = p1;
    data_[1] = p2;
    data_[2] = p3;
  }

  const Point& p1() const { return data_[0]; }
  const Point& p2() const { return data_[1]; }
  const Point& p3() const { return data_[2]; }

  const Point* data() const { return data_; }
  const Point& operator[](size_t i) const { return data_[i]; }

  void set_p1(const Point& p1) { data_[0] = p1; }
  void set_p2(const Point& p2) { data_[1] = p2; }
  void set_p3(const Point& p3) { data_[2] = p3; }

  Triangle& FlipNormal();

  // Returns the plane that contains this triangle.
  Plane GetPlane() const;

 private:
  Point data_[3];
};

}  // namespace vfx

#endif  // VFX_GEOMETRY_TRIANGLE_H_
