// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RSTRANSFORM_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RSTRANSFORM_H_

#include <algorithm>
#include <limits>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_rect.h"
#include "flutter/display_list/geometry/dl_transform.h"
#include "fml/logging.h"

namespace flutter {

/// A struct representing a transform for an image consisting of a rotation,
/// a uniform scale (in X and Y), and a translation.
///
/// NaN and Infinity handling
///
/// The factories will normalize any NaN or Infinity values in the angle or
/// sin/cos pair to a scale of 1 and an angle of 0 degrees/radians because
/// it is impossible to determine the orientation of the transformed image.
///
/// The factories will also normalize any translation component that is a
/// NaN value to a value of 0. Infinity values are allowed in the translation
/// components but will likely result in the resulting transformed image being
/// clipped.
struct DlRSTransform {
 private:
  DlFVector scaled_trig_;
  DlFPoint origin_;

 public:
  DlRSTransform() : scaled_trig_(1.0f, 0.0f), origin_(0.0f, 0.0f) {}
  DlRSTransform(const DlRSTransform& rst) = default;
  DlRSTransform(DlRSTransform&& rst) = default;

  static DlRSTransform MakeScaledCosSinXY(DlScalar scos,
                                          DlScalar ssin,
                                          DlScalar tx,
                                          DlScalar ty) {
    if (!DlScalars_AreFinite(scos, ssin)) {
      scos = 1.0f;
      ssin = 0.0f;
    }
    if (DlScalar_IsNaN(tx)) {
      tx = 0.0f;
    }
    if (DlScalar_IsNaN(ty)) {
      ty = 0.0f;
    }
    return {scos, ssin, tx, ty};
  }

  static DlRSTransform MakeScaleAngleXY(DlScalar scale,
                                        DlAngle angle,
                                        DlScalar tx,
                                        DlScalar ty) {
    // DlAngle is self-correcting for non-finite values
    FML_DCHECK(DlScalar_IsFinite(angle.radians()));
    if (!DlScalar_IsFinite(scale)) {
      scale = 1.0f;
      angle = DlAngle();
    }
    if (DlScalar_IsNaN(tx)) {
      tx = 0.0f;
    }
    if (DlScalar_IsNaN(ty)) {
      ty = 0.0f;
    }
    DlFVector scos_ssin = angle.CosSin() * scale;
    return {scos_ssin.x(), scos_ssin.y(), tx, ty};
  }

  inline DlScalar scaled_cos() const { return scaled_trig_.x(); }
  inline DlScalar scaled_sin() const { return scaled_trig_.y(); }
  inline DlScalar translate_x() const { return origin_.x(); }
  inline DlScalar translate_y() const { return origin_.y(); }

  /// Attempts to extract the scale used to construct this |DlRSTransform|.
  /// Note that negative scales are indistinguishable from 180 degree
  /// rotations so the returned answer will always be non-negative and
  /// any negative uniform scale used in constructing the object will be
  /// accounted for as a 180 degree rotation in |ExtractAngle|.
  DlScalar ExtractScale() const { return scaled_trig_.GetLength(); }

  /// Attempts to extract the scale used to construct this |DlTransform|.
  /// The return value will be normalized to the half-open interval:
  /// - (-180, +180] degrees, or
  /// - (-Pi, +Pi] radians
  /// A negative scale used to construct this transform would cause the
  /// result of this angle extraction to be off by 180 degrees since it
  /// would be indistinguishable from the reversed angle.
  DlAngle ExtractAngle() const;

  bool operator==(const DlRSTransform& other) const {
    return scaled_trig_ == other.scaled_trig_ && origin_ == other.origin_;
  }
  bool operator!=(const DlRSTransform& other) const {
    return !(*this == other);
  }

  bool IsIdentity() const {
    return scaled_trig_.x() == 1.0f &&  //
           scaled_trig_.y() == 0.0f &&  //
           origin_.x() == 0.0f &&       //
           origin_.y() == 0.0f;
  }

  bool IsFinite() const {
    return scaled_trig_.IsFinite() && origin_.IsFinite();
  }

  // Returns the 4 corners of the parallelogram transformed from Rect(0,0,w,h)
  // in the order defined by going clockwise around the untransformed rect.
  void ToQuad(DlScalar width, DlScalar height, DlFPoint quad[4]) const;
  void ToQuad(DlFSize size, DlFPoint quad[4]) const {
    return ToQuad(size.width(), size.height(), quad);
  }

 private:
  DlRSTransform(DlScalar scos, DlScalar ssin, DlScalar tx, DlScalar ty)
      : scaled_trig_(scos, ssin), origin_(tx, ty) {
    FML_DCHECK(scaled_trig_.IsFinite());
    FML_DCHECK(!DlScalar_IsNaN(origin_.x()));
    FML_DCHECK(!DlScalar_IsNaN(origin_.y()));
  }
};

std::ostream& operator<<(std::ostream& os, const DlRSTransform& rst);

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RSTRANSFORM_H_
