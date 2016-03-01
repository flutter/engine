// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GEOMETRY_QUAD_H_
#define VFX_GEOMETRY_QUAD_H_

#include <memory>

#include "vfx/geometry/point.h"
#include "vfx/geometry/plane.h"

namespace vfx {
class Offset;

// A region bounded by four lines in a plane.
class Quad {
 public:
  Quad() { memset(data_, 0, sizeof(data_)); }
  explicit Quad(Point data[4]) { memcpy(data_, data, sizeof(data_)); }
  Quad(const Point& p1, const Point& p2, const Point& p3, const Point& p4) {
    data_[0] = p1;
    data_[1] = p2;
    data_[2] = p3;
    data_[3] = p4;
  }

  const Point& p1() const { return data_[0]; }
  const Point& p2() const { return data_[1]; }
  const Point& p3() const { return data_[2]; }
  const Point& p4() const { return data_[3]; }

  const Point* data() const { return data_; }
  const Point& operator[](size_t i) const { return data_[i]; }

  void set_p1(const Point& p1) { data_[0] = p1; }
  void set_p2(const Point& p2) { data_[1] = p2; }
  void set_p3(const Point& p3) { data_[2] = p3; }
  void set_p4(const Point& p4) { data_[3] = p4; }

  void Move(const Offset& offset);
  Quad ProjectDistanceFromSource(const Point& source, double distance) const;

  Offset GetNormal() const;
  Offset GetUnitNormal() const;

  // Returns the plane that contains this quad.
  //
  // Because quads use floating point, it's unlikely that a quad actually lies
  // in a mathematical plane. This function actually returns the plane that
  // contains p1, p2, and p3.
  Plane GetPlane() const;

 private:
  Point data_[4];
};

}  // namespace vfx

#endif  // VFX_GEOMETRY_QUAD_H_
