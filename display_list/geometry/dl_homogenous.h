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

// The minimum practical value for a homogenous coordinate before
// the point is considered clipped by the viewing plane.
static constexpr DlScalar kMinimumHomogenous = 1.0 / (1 << 14);

template <typename T>
struct DlTHomogenous3D {
 private:
  static constexpr T zero_ = static_cast<T>(0);
  static constexpr T one_ = static_cast<T>(1);

  T x_;
  T y_;
  T z_;
  T w_;

 public:
  constexpr DlTHomogenous3D() : DlTHomogenous3D(0, 0) {}
  constexpr DlTHomogenous3D(T x, T y, T z = zero_, T w = one_)
      : x_(x), y_(y), z_(z), w_(w) {}
  constexpr explicit DlTHomogenous3D(const DlTPoint<T> p)
      : DlTHomogenous3D(p.x(), p.y()) {}

  inline T x() const { return x_; }
  inline T y() const { return y_; }
  inline T z() const { return z_; }
  inline T w() const { return w_; }

  [[nodiscard]] DlTPoint<T> normalizedToPoint() const {
    T inv_w = DlScalar_IsNearlyZero(w_) ? one_ : one_ / w_;
    return {x_ * inv_w, y_ * inv_w};
  }

  [[nodiscard]] DlTHomogenous3D normalized() const {
    T inv_w = DlScalar_IsNearlyZero(w_) ? one_ : one_ / w_;
    return {x_ * inv_w, y_ * inv_w, z_ * inv_w};
  }

  bool operator==(const DlTHomogenous3D& p) const {
    return x_ == p.x_ && y_ == p.y_ && z_ == p.z_ && w_ == p.w_;
  }
  bool operator!=(const DlTHomogenous3D& p) const { return !(*this == p); }

  bool is_finite() const { return DlScalars_AreAllFinite(&x_, 4); }

  bool is_unclipped() const { return w_ >= kMinimumHomogenous; }
};

using DlFHomogenous3D = DlTHomogenous3D<DlScalar>;

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_HOMOGENOUS_H_
