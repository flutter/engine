// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_HOMOGENOUS_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_HOMOGENOUS_H_

#include <algorithm>
#include <limits>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/geometry/dl_point.h"

namespace flutter {

template <typename T>
struct DlTHomogenous2D {
 private:
  T x_;
  T y_;
  T w_;

 public:
  // The minimum practical value for a homogenous coordinate before
  // the point is considered clipped by the viewing plane.
  static constexpr DlScalar kMinimumHomogenous = 1.0 / (1 << 14);

  constexpr DlTHomogenous2D() : DlTHomogenous2D(0, 0, 0) {}
  constexpr DlTHomogenous2D(T x, T y) : x_(x), y_(y), w_(1.0f) {}
  constexpr DlTHomogenous2D(DlTPoint<T> p) : x_(p.x()), y_(p.y()), w_(1.0f) {}
  constexpr DlTHomogenous2D(T x, T y, T w) : x_(x), y_(y), w_(w) {}

  explicit constexpr DlTHomogenous2D(const DlTPoint<T>& p)
      : DlTHomogenous2D(p.x, p.y) {}

  inline T x() { return x_; }
  inline T y() { return y_; }
  inline T w() { return w_; }

  DlTPoint<T> normalize() const {
    DlScalar inv_w = DlScalar_IsNearlyZero(w_) ? 1.0f : 1.0f / w_;
    return {x_ * inv_w, y_ * inv_w};
  }

  bool operator==(const DlTHomogenous2D& p) const {
    return x_ == p.x_ && y_ == p.y_ && w_ == p.w_;
  }
  bool operator!=(const DlTHomogenous2D& p) const { return !(*this == p); }

  bool is_finite() const { return DlScalars_AreAllFinite(&x_, 3); }

  bool is_unclipped() const { return w_ >= kMinimumHomogenous; }
};

using DlFHomogenous2D = DlTHomogenous2D<DlScalar>;

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_HOMOGENOUS_H_
