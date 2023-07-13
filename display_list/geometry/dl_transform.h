// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_TRANSFORM_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_TRANSFORM_H_

#include <algorithm>
#include <limits>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/geometry/dl_rect.h"

namespace flutter {

class DlTransform {
 public:
  DlTransform() : DlTransform(  //
      1.0f, 0.0f, 0.0f, 0.0f,   //
      0.0f, 1.0f, 0.0f, 0.0f,   //
      0.0f, 0.0f, 1.0f, 0.0f,   //
      0.0f, 0.0f, 0.0f, 1.0f,   //
      Complexity::kIdentity) {}

  DlTransform Translate(DlScalar tx, DlScalar ty) {
    return {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
          tx,   ty, 0.0f, 1.0f,
        Complexity::kTranslate
    };
  }

  DlTransform Scale(DlScalar sx, DlScalar sy) {
    return {
          sx, 0.0f, 0.0f, 0.0f,
        0.0f,   sy, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        Complexity::kScaleTranslate
    };
  }

  DlScalar rc(int row, int col) {
    return m_[col * 4 + row];
  }

  DlRectF TransformRect(const DlRectF& rect) const;

 private:
  enum class Complexity {
    kUnknown,
    kIdentity,
    kTranslate,
    kScaleTranslate,
    kAffine2D,
    kComplex,
  };

  Complexity complexity() const;

  // Internal format and private constructor are column-major for ease of
  // transform implementation.
  DlTransform(DlScalar mxx, DlScalar myx, DlScalar mzx, DlScalar mwx,
              DlScalar mxy, DlScalar myy, DlScalar mzy, DlScalar mwy,
              DlScalar mxz, DlScalar myz, DlScalar mzz, DlScalar mwz,
              DlScalar mxt, DlScalar myt, DlScalar mzt, DlScalar mwt,
              Complexity complexity)
      : m_{
          mxx, myx, mzx, mwx,
          mxy, myy, mzy, mwy,
          mxz, myz, mzz, mwz,
          mxt, myt, mzt, mwt,
      }, complexity_(complexity) {}

  DlScalar m_[16];
  mutable Complexity complexity_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_TRANSFORM_H_
