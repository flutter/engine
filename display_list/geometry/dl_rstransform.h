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

namespace flutter {

struct DlRSTransform {
 private:
  DlFVector trig_;
  DlFPoint origin_;

 public:
  DlRSTransform() : DlRSTransform(1.0f, 0.0f, 0.0f, 0.0f) {}

  static DlRSTransform MakeScaledCosSinXY(DlScalar scos,
                                          DlScalar ssin,
                                          DlScalar tx,
                                          DlScalar ty) {
    return {scos, ssin, tx, ty};
  }

  static DlRSTransform MakeScaleAngleXY(DlScalar scale,
                                        DlAngle angle,
                                        DlScalar tx,
                                        DlScalar ty) {
    DlFVector scos_ssin = angle.CosSin() * scale;
    return {scos_ssin.x(), scos_ssin.y(), tx, ty};
  }

  inline DlScalar scaled_cos() const { return trig_.x(); }
  inline DlScalar scaled_sin() const { return trig_.y(); }
  inline DlScalar translate_x() const { return origin_.x(); }
  inline DlScalar translate_y() const { return origin_.y(); }

  DlScalar ExtractScale() const { return trig_.Length(); }
  DlAngle ExtractAngle() const;

  bool operator==(const DlRSTransform& other) const {
    return trig_ == other.trig_ && origin_ == other.origin_;
  }

  bool operator!=(const DlRSTransform& other) const {
    return !(*this == other);
  }

  bool IsIdentity() const {
    return trig_.x() == 1.0f &&    //
           trig_.y() == 0.0f &&    //
           origin_.x() == 0.0f &&  //
           origin_.y() == 0.0f;
  }

  bool IsFinite() const { return trig_.IsFinite() && origin_.IsFinite(); }

  // Returns the 4 corners of the parallelogram transformed from Rect(0,0,w,h)
  // in the order defined by going clockwise around the untransformed rect.
  void ToQuad(DlScalar width, DlScalar height, DlFVector quad[4]) const;
  void ToQuad(DlFSize size, DlFVector quad[4]) const {
    return ToQuad(size.width(), size.height(), quad);
  }

 private:
  DlRSTransform(DlScalar scos, DlScalar ssin, DlScalar tx, DlScalar ty)
      : trig_(scos, ssin), origin_(tx, ty) {}
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RSTRANSFORM_H_
