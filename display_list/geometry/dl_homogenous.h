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

/// The minimum practical value for a homogenous coordinate before
/// the point is considered clipped by the viewing plane.
static constexpr DlScalar kDlMinimumHomogenous = 1.0 / (1 << 14);

/// A struct for storing a 3D homogenous coordinate. Homogenous coordinates
/// are scaled by a common weight factor and result naturally from transform
/// operations in perspective coordinate systems. The |x|, |y|, and |z|
/// coordinates are not immediately related to a coordinate in the 3D space
/// but must first be divided by the |w| weight property to be a valid
/// coordinate. Thus, the true 3D point represented by this homogemous
/// point is: {X, Y, Z} = { x() / w(), y() / w(), z() / w() }
///
/// Most homogenous coordinates will start out with a default weight of
/// "1.0" and the weight property will only be modified by transforms
/// that include perspective (usually resulting from rotations around
/// axes other than the Z axis).
///
/// Points with negative or zero weights are typically considered clipped
/// by the near plane in a 3D coordinate system. The |kDlMinimumHomogenous|
/// constant specifies a small practical positive value which is considered
/// the minimum visible unclipped weight in order to prevent floating point
/// overflow when a coordinate is normalized.
///
/// Points may be left in homogenous form while undergoing a number of
/// transform, or transform-related, operations until a final coordinate
/// is needed at which point they can be normalized manually, or via the
/// |Normalize| method into an absolute 3D coordinate. Since sequential
/// transform operations might move a point behind and then back in front
/// of the near plane, it is recommended not to normalize a point until
/// you are done with all transforms and need to treat it like an absolute
/// coordinate (i.e. render it). While coordinates behind the near clipping
/// plane cannot be represented in most traditional 3D spaces, such
/// coordinates should not simply be ignored, but instead some interpolation
/// should be performed to track the overall geometry from any points that
/// appear inside the clipping fulcrum to the points behind the near plane
/// and substituting the locations where the geometry passes through that
/// clipping plane.
///
/// Transforms that do not affect perspective will leave the homogenous
/// weight property of all points as "1.0" and so tracking the homogenous
/// weight can be inefficient unless a transform will involve perspective,
/// but is otherwise safe as the default weight of "1.0" safely produces
/// normalized absolute coordinates that match the homogenous coordinates.
///
/// NaN and Infinity handling
///
/// The structure will happily store NaN and Infinity values, but will
/// normalize to a zero vector if any are present in the source vector.
struct DlFHomogenous3D {
 private:
  DlScalar x_;
  DlScalar y_;
  DlScalar z_;
  DlScalar w_;

 public:
  constexpr DlFHomogenous3D() : DlFHomogenous3D(0.0f, 0.0f) {}
  constexpr DlFHomogenous3D(DlScalar x,
                            DlScalar y,
                            DlScalar z = 0.0f,
                            DlScalar w = 1.0f)
      : x_(x), y_(y), z_(z), w_(w) {}
  constexpr explicit DlFHomogenous3D(const DlTPoint<DlScalar> p)
      : DlFHomogenous3D(p.x(), p.y()) {}

  /// Returns the un-normalized homogenous X value
  inline DlScalar x() const { return x_; }
  /// Returns the un-normalized homogenous Y value
  inline DlScalar y() const { return y_; }
  /// Returns the un-normalized homogenous Z value
  inline DlScalar z() const { return z_; }
  /// Returns the un-normalized homogenous W value
  inline DlScalar w() const { return w_; }

  [[nodiscard]] DlTPoint<DlScalar> NormalizeToPoint() const {
    if (!IsFinite()) {
      return {};
    }
    DlScalar inv_w = DlScalar_IsNearlyZero(w_) ? 1.0f : 1.0f / w_;
    return {x_ * inv_w, y_ * inv_w};
  }

  [[nodiscard]] DlFHomogenous3D Normalize() const {
    if (!IsFinite()) {
      return {};
    }
    DlScalar inv_w = DlScalar_IsNearlyZero(w_) ? 1.0f : 1.0f / w_;
    return {x_ * inv_w, y_ * inv_w, z_ * inv_w};
  }

  bool operator==(const DlFHomogenous3D& p) const {
    return x_ == p.x_ && y_ == p.y_ && z_ == p.z_ && w_ == p.w_;
  }
  bool operator!=(const DlFHomogenous3D& p) const { return !(*this == p); }

  bool IsFinite() const { return DlScalars_AreAllFinite(&x_, 4); }

  bool IsUnclipped() const { return w_ >= kDlMinimumHomogenous; }
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_HOMOGENOUS_H_
