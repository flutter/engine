// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GEOMETRY_POINT_H_
#define VFX_GEOMETRY_POINT_H_

#include <memory>

#include "vfx/geometry/offset.h"

namespace vfx {

// A point in three dimensions.
class Point {
 public:
  Point() { memset(data_, 0, sizeof(data_)); }
  explicit Point(float data[3]) { memcpy(data_, data, sizeof(data_)); }
  Point(float x, float y, float z) {
    data_[0] = x;
    data_[1] = y;
    data_[2] = z;
  }

  float x() const { return data_[0]; }
  float y() const { return data_[1]; }
  float z() const { return data_[2]; }

  const float* data() const { return data_; }

  void set_x(float x) { data_[0] = x; }
  void set_y(float y) { data_[1] = y; }
  void set_z(float z) { data_[2] = z; }

  void Move(const Offset& offset);

 private:
  float data_[3];
};

inline Offset operator-(const Point& a, const Point& b) {
  return Offset(a.x() - b.x(), a.y() - b.y(), a.z() - b.z());
}

inline Point operator-(const Point& base, const Offset& offset) {
  return Point(base.x() - offset.dx(),
               base.y() - offset.dy(),
               base.z() - offset.dz());
}

inline Point operator+(const Point& base, const Offset& offset) {
  return Point(base.x() + offset.dx(),
               base.y() + offset.dy(),
               base.z() + offset.dz());
}

}  // namespace vfx

#endif  // VFX_GEOMETRY_POINT_H_
