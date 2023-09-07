// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_transform.h"
#include "flutter/display_list/utils/dl_bounds_accumulator.h"

#include "flutter/fml/logging.h"

namespace flutter {

DlTransform DlTransform::MakeConcat(const DlTransform& outer,
                                    const DlTransform& inner) {
  if (outer.complexity() <= Complexity::kIdentity) {
    return inner;
  }
  if (inner.complexity() <= Complexity::kIdentity) {
    return outer;
  }
  DlScalar new_m[16];
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      double v = 0.0f;
      for (int i = 0; i < 4; i++) {
        double iv = inner.m_[r * 4 + i];
        double ov = outer.m_[i * 4 + c];
        v += iv * ov;
      }
      new_m[r * 4 + c] = static_cast<DlScalar>(v);
    }
  }
  return DlTransform::MakeColMajor(new_m);
}

DlTransform::Complexity DlTransform::ComputeComplexity(const DlScalar m[16]) {
  Complexity ret = Complexity::kIdentity;
  for (int i = 0; i < 16; i++) {
    if (m[i] != identity_values[i]) {
      ret = std::max(ret, complexity_values[i]);
      if (ret >= Complexity::kLast) {
        break;
      }
    }
  }
  return ret;
}

bool DlTransform::RectStaysRect() const {
  switch (complexity()) {
    case Complexity::kIdentity:
    case Complexity::kTranslate2D:
    case Complexity::kScaleTranslate2D:
      return true;

    case Complexity::kAffine2D:
    case Complexity::kAffine3D:
      return (m_[kXY] == 0.0f && m_[kYX] == 0.0f) ||
             (m_[kXX] == 0.0f && m_[kYY] == 0.0f);

    case Complexity::kUnknown:
      FML_DCHECK(complexity_ != Complexity::kUnknown);
    case Complexity::kPerspectiveOnlyZ:
    case Complexity::kPerspectiveAll:
      return false;
  }
}

DlTransform& DlTransform::TranslateInner(DlScalar tx, DlScalar ty) {
  if (!DlScalars_AreFinite(tx, ty) || (tx == 0.0f && ty == 0.0f)) {
    return *this;
  }
  switch (complexity()) {
    case Complexity::kIdentity:
    case Complexity::kTranslate2D:
      m_[kXT] += tx;
      m_[kYT] += ty;
      complexity_ = (m_[kXT] == 0 && m_[kYT] == 0) ? Complexity::kIdentity
                                                   : Complexity::kTranslate2D;
      break;

    case Complexity::kScaleTranslate2D:
      m_[kXT] += m_[kXX] * tx;  // + 0 * ty
      m_[kYT] += m_[kYY] * ty;  // + 0 * tx
      // Whether or not translation goes away, we will still be ScaleTranslate
      break;

    case Complexity::kAffine2D:
      m_[kXT] += m_[kXX] * tx + m_[kXY] * ty;
      m_[kYT] += m_[kYX] * tx + m_[kYY] * ty;
      // Whether or not translation goes away, we will still be Affine
      break;

    case Complexity::kAffine3D:
      m_[kXT] += m_[kXX] * tx + m_[kXY] * ty;
      m_[kYT] += m_[kYX] * tx + m_[kYY] * ty;
      m_[kZT] += m_[kZX] * tx + m_[kZY] * ty;
      // Whether or not translation goes away, we will still be Affine
      break;

    case Complexity::kUnknown:
      FML_DCHECK(complexity_ != Complexity::kUnknown);
    case Complexity::kPerspectiveOnlyZ:
    case Complexity::kPerspectiveAll:
      DlFHomogenous3D transformed_tx = TransformHomogenous(tx, ty);
      DlFPoint normalized_tx = transformed_tx.NormalizeToPoint();
      m_[kXT] += normalized_tx.x();
      m_[kYT] += normalized_tx.y();
      // Whether or not translation goes away, we will still be Perspective
      break;
  }
  return *this;
}

DlTransform& DlTransform::TranslateOuter(DlScalar tx, DlScalar ty) {
  if (!DlScalars_AreFinite(tx, ty) || (tx == 0.0f && ty == 0.0f)) {
    return *this;
  }
  m_[kXT] += tx;
  m_[kYT] += ty;
  if (complexity() <= Complexity::kTranslate2D) {
    complexity_ = (m_[kXT] == 0 && m_[kYT] == 0) ? Complexity::kIdentity
                                                 : Complexity::kTranslate2D;
  }  // else anything higher and we stay the same regardless of tx/ty
  return *this;
}

DlTransform& DlTransform::ScaleInner(DlScalar sx, DlScalar sy) {
  return ConcatInner(MakeScale(sx, sy));
}
DlTransform& DlTransform::ScaleOuter(DlScalar sx, DlScalar sy) {
  return ConcatOuter(MakeScale(sx, sy));
}

DlTransform& DlTransform::SkewInner(DlScalar sx, DlScalar sy) {
  return ConcatInner(MakeSkew(sx, sy));
}
DlTransform& DlTransform::SkewOuter(DlScalar sx, DlScalar sy) {
  return ConcatOuter(MakeSkew(sx, sy));
}

DlTransform& DlTransform::RotateInner(const DlAngle& angle) {
  return ConcatInner(MakeRotate(angle));
}
DlTransform& DlTransform::RotateOuter(const DlAngle& angle) {
  return ConcatOuter(MakeRotate(angle));
}

DlTransform& DlTransform::RotateInner(DlFVector3 axis, const DlAngle& angle) {
  return ConcatInner(MakeRotate(axis, angle));
}
DlTransform& DlTransform::RotateOuter(DlFVector3 axis, const DlAngle& angle) {
  return ConcatOuter(MakeRotate(axis, angle));
}

DlTransform& DlTransform::ConcatInner(const DlTransform& inner) {
  return *this = MakeConcat(*this, inner);
}
DlTransform& DlTransform::ConcatOuter(const DlTransform& outer) {
  return *this = MakeConcat(outer, *this);
}

DlFPoint DlTransform::TransformPoint(DlScalar x, DlScalar y) const {
  if (!DlScalars_AreFinite(x, y)) {
    return {};
  }
  switch (complexity()) {
    case Complexity::kIdentity:
      return DlFPoint(x, y);

    case Complexity::kTranslate2D:
      return DlFPoint(x + m_[12], y + m_[13]);

    case Complexity::kScaleTranslate2D:
      return DlFPoint(x * m_[kXX] + m_[kXT],  //
                      y * m_[kYY] + m_[kYT]);

    case Complexity::kPerspectiveOnlyZ:  // Perspective only on Z (which is 0)
    case Complexity::kAffine3D:          // We don't care about Z in or out
    case Complexity::kAffine2D:
      return DlFPoint(x * m_[kXX] + y * m_[kXY] + m_[kXT],
                      x * m_[kYX] + y * m_[kYY] + m_[kYT]);

    case Complexity::kUnknown:
      FML_DCHECK(complexity_ != Complexity::kUnknown);
    case Complexity::kPerspectiveAll: {
      DlScalar w = x * m_[kWX] + y * m_[kWY] + m_[kWT];
      if (w < kDlMinimumHomogenous) {
        return DlFPoint();
      }
      w = 1.0 / w;
      return DlFPoint(w * (x * m_[kXX] + y * m_[kXY] + m_[kXT]),
                      w * (x * m_[kYX] + y * m_[kYY] + m_[kYT]));
    }
  }
}

void DlTransform::TransformPoints(DlFPoint dst[],
                                  const DlFPoint src[],
                                  int count) const {
  switch (complexity()) {
    case Complexity::kIdentity:
      memcpy(dst, src, count * sizeof(DlFPoint));
      break;

    case Complexity::kTranslate2D: {
      DlScalar tx = m_[kXT];
      DlScalar ty = m_[kYT];
      for (int i = 0; i < count; i++) {
        DlFPoint p = src[i];
        dst[i] = DlFPoint(p.x() + tx, p.y() + ty);
      }
      break;
    }

    case Complexity::kScaleTranslate2D: {
      DlScalar mXX = m_[kXX];
      DlScalar mYY = m_[kYY];
      DlScalar tx = m_[kXT];
      DlScalar ty = m_[kYT];
      for (int i = 0; i < count; i++) {
        DlFPoint p = src[i];
        dst[i] = DlFPoint(mXX * p.x() + tx, mYY * p.y() + ty);
      }
      break;
    }

    case Complexity::kPerspectiveOnlyZ:  // Perspective only on Z (which is 0)
    case Complexity::kAffine3D:          // We don't care about Z in or out
    case Complexity::kAffine2D: {
      DlScalar mXX = m_[kXX];
      DlScalar mXY = m_[kXY];
      DlScalar mYX = m_[kYX];
      DlScalar mYY = m_[kYY];
      DlScalar tx = m_[kXT];
      DlScalar ty = m_[kYT];
      for (int i = 0; i < count; i++) {
        DlFPoint p = src[i];
        DlScalar x = p.x();
        DlScalar y = p.y();
        dst[i] = DlFPoint(mXX * x + mXY * y + tx,  //
                          mYX * x + mYY * y + ty);
      }
      break;
    }

    case Complexity::kUnknown:
      FML_DCHECK(complexity_ != Complexity::kUnknown);
    case Complexity::kPerspectiveAll: {
      DlScalar mXX = m_[kXX];
      DlScalar mXY = m_[kXY];
      DlScalar mYX = m_[kYX];
      DlScalar mYY = m_[kYY];
      DlScalar mWX = m_[kWX];
      DlScalar mWY = m_[kWY];
      DlScalar tx = m_[kXT];
      DlScalar ty = m_[kYT];
      DlScalar tw = m_[kWT];
      for (int i = 0; i < count; i++) {
        DlFPoint p = src[i];
        DlScalar x = p.x();
        DlScalar y = p.y();
        DlScalar w = mWX * x + mWY * y + tw;
        if (w < kDlMinimumHomogenous) {
          dst[i] = DlFPoint(0, 0);
        } else {
          w = 1.0 / w;
          dst[i] = DlFPoint(w * (mXX * x + mXY * y + tx),
                            w * (mYX * x + mYY * y + ty));
        }
      }
      break;
    }
  }
}

DlFHomogenous3D DlTransform::TransformHomogenous(DlScalar x,
                                                 DlScalar y,
                                                 DlScalar z,
                                                 DlScalar w) const {
  if (!DlScalars_AreFinite(x, y)) {
    return {};
  }
  switch (complexity()) {
    case Complexity::kIdentity:
      return DlFHomogenous3D(x, y, z, w);

    case Complexity::kTranslate2D:
      return DlFHomogenous3D(x + w * m_[kXT], y + w * m_[kYT], z, w);

    case Complexity::kScaleTranslate2D:
      return DlFHomogenous3D(x * m_[kXX] + w * m_[kXT],
                             y * m_[kYY] + w * m_[kYT], z, w);

    case Complexity::kAffine2D:
      return DlFHomogenous3D(x * m_[kXX] + y * m_[kXY] + w * m_[kXT],
                             x * m_[kYX] + y * m_[kYY] + w * m_[kYT], z, w);

    case Complexity::kAffine3D:
      return DlFHomogenous3D(
          x * m_[kXX] + y * m_[kXY] + z * m_[kXZ] + w * m_[kXT],
          x * m_[kYX] + y * m_[kYY] + z * m_[kYZ] + w * m_[kYT],
          x * m_[kZX] + y * m_[kZY] + z * m_[kZZ] + w * m_[kZT], w);

    case Complexity::kUnknown:
      FML_DCHECK(complexity_ != Complexity::kUnknown);

    case Complexity::kPerspectiveOnlyZ:
    case Complexity::kPerspectiveAll: {
      return DlFHomogenous3D(
          x * m_[kXX] + y * m_[kXY] + z * m_[kXZ] + w * m_[kXT],
          x * m_[kYX] + y * m_[kYY] + z * m_[kYZ] + w * m_[kYT],
          x * m_[kZX] + y * m_[kZY] + z * m_[kZZ] + w * m_[kZT],
          x * m_[kWX] + y * m_[kWY] + z * m_[kWZ] + w * m_[kWT]);
    }
  }
}

DlFRect DlTransform::TransformRect(const DlFRect& rect) const {
  if (rect.IsEmpty() || !rect.IsFinite()) {
    return {};
  }
  switch (complexity()) {
    case Complexity::kIdentity:
      return rect;

    case Complexity::kTranslate2D:
      return DlFRect::MakeLTRB(rect.left() + m_[kXT],   //
                               rect.top() + m_[kYT],    //
                               rect.right() + m_[kXT],  //
                               rect.bottom() + m_[kYT]);

    case Complexity::kScaleTranslate2D: {
      DlFPoint ul = TransformPoint(rect.left(), rect.top());
      DlFPoint lr = TransformPoint(rect.right(), rect.bottom());
      return DlFRect::MakeLTRB(      //
          std::min(ul.x(), lr.x()),  //
          std::min(ul.y(), lr.y()),  //
          std::max(ul.x(), lr.x()),  //
          std::max(ul.y(), lr.y())   //
      );
    }

    case Complexity::kPerspectiveOnlyZ:  // Perspective only on Z (which is 0)
    case Complexity::kAffine3D:          // We don't care about Z in or out
    case Complexity::kAffine2D: {
      DlFPoint ul = TransformPoint(rect.left(), rect.top());
      DlFPoint ur = TransformPoint(rect.right(), rect.top());
      DlFPoint ll = TransformPoint(rect.left(), rect.bottom());
      DlFPoint lr = TransformPoint(rect.right(), rect.bottom());
      return DlFRect::MakeLTRB(
          std::min(std::min(ul.x(), ur.x()), std::min(ll.x(), lr.x())),
          std::min(std::min(ul.y(), ur.y()), std::min(ll.y(), lr.y())),
          std::max(std::max(ul.x(), ur.x()), std::max(ll.x(), lr.x())),
          std::max(std::max(ul.y(), ur.y()), std::max(ll.y(), lr.y())));
    }

    case Complexity::kUnknown:
      FML_DCHECK(complexity_ != Complexity::kUnknown);

    case Complexity::kPerspectiveAll: {
      DlFHomogenous3D ul = TransformHomogenous(rect.left(), rect.top());
      DlFHomogenous3D ur = TransformHomogenous(rect.right(), rect.top());
      DlFHomogenous3D ll = TransformHomogenous(rect.left(), rect.bottom());
      DlFHomogenous3D lr = TransformHomogenous(rect.right(), rect.bottom());

      RectBoundsAccumulator accumulator;

      // Process the homogenous coordinate of the corner of the transformed
      // rectangle. If the transformed point is unclipped, it is added to the
      // accumulator directly. If it is clipped by the near clipping plane
      // as determined by |kDlMinimumHomogenous| then it will be linearly
      // interpolated towards the transformed coordinates of its 2 adjacent
      // corners and the point at which that side of the quad is clipped
      // will be included in the bounds.
      auto ProcessCorner = [&accumulator](const DlFHomogenous3D& previous,
                                          const DlFHomogenous3D& p,
                                          const DlFHomogenous3D& next) {
        if (!p.IsFinite()) {
          return;
        }
        if (p.IsUnclipped()) {
          accumulator.accumulate(p.NormalizeToPoint());
          return;
        }
        // Process a single corner against a single neighbor. The
        // point |p| is assumed to be clipped and the neighbor |n|
        // is only processed if it is unclipped. We linearly interpolate
        // to find the point at which the edge between them goes
        // out of bounds against the near clipping plane.
        auto InterpolateAndProcess = [&accumulator](const DlFHomogenous3D& p,
                                                    const DlFHomogenous3D& n) {
          FML_DCHECK(p.IsFinite() && !p.IsUnclipped());
          if (n.IsFinite() && n.IsUnclipped()) {
            DlScalar fract = (kDlMinimumHomogenous - p.w()) / (n.w() - p.w());
            DlScalar x = n.x() * fract + p.x() * (1.0f - fract);
            DlScalar y = n.y() * fract + p.y() * (1.0f - fract);
            accumulator.accumulate(x / kDlMinimumHomogenous,
                                   y / kDlMinimumHomogenous);
          }
        };
        // Either, both, or neither of these interpolations may lead to a
        // point being added to the bounds, depending on whether just the
        // corner is clipped (2 get added), or one of the adjacent edges
        // of the quad is entirely clipped (1 gets added), or all 3 are
        // clipped and hopefully the remaining 4th point is unclipped (in
        // which case no points are accumulated based on these 3 points).
        InterpolateAndProcess(p, previous);
        InterpolateAndProcess(p, next);
      };
      // Process all 4 corners along with their neighbors, moving clockwise
      // around the quad. (Note that the order doesn't matter as long as
      // the 2 neighbors are supplied for each corner, but going clockwise
      // makes it easier to follow the logic.)
      ProcessCorner(ll, ul, ur);
      ProcessCorner(ul, ur, lr);
      ProcessCorner(ur, lr, ll);
      ProcessCorner(lr, ll, ul);
      return accumulator.bounds();
    }
  }
}

DlScalar DlTransform::Determinant() const {
  switch (complexity()) {
    case Complexity::kIdentity:
    case Complexity::kTranslate2D:
      return kDlScalar_One;

    case Complexity::kScaleTranslate2D:
      return m_[kXX] * m_[kYY];

    case Complexity::kAffine2D:
      return m_[kXX] * m_[kYY] - m_[kXY] * m_[kYX];

    case Complexity::kUnknown:
      FML_DCHECK(complexity_ != Complexity::kUnknown);

    case Complexity::kAffine3D:
    case Complexity::kPerspectiveOnlyZ:
    case Complexity::kPerspectiveAll:
      return Invert4x4Matrix(m_, nullptr);
  }
}

std::optional<DlTransform> DlTransform::Inverse() const {
  switch (complexity()) {
    case Complexity::kIdentity:
      return DlTransform();

    case Complexity::kTranslate2D: {
      DlScalar tx = m_[kXT];
      DlScalar ty = m_[kYT];
      if (!DlScalar_IsNaN(tx) && !DlScalar_IsNaN(ty)) {
        return DlTransform::MakeTranslate(-tx, -ty);
      }
      break;
    }

    case Complexity::kScaleTranslate2D: {
      double sx = 1.0 / m_[kXX];
      double sy = 1.0 / m_[kYY];
      double tx = m_[kXT];
      double ty = m_[kYT];
      if (!DlScalar_IsNaN(sx) && !DlScalar_IsNaN(sy) &&  //
          !DlScalar_IsNaN(tx) && !DlScalar_IsNaN(ty)) {
        return DlTransform::MakeScale(sx, sy)  //
            .TranslateInner(-tx, -ty);
      }
      break;
    }

    case Complexity::kAffine2D: {
      double sxx = m_[kXX];
      double syx = m_[kYX];
      double sxy = m_[kXY];
      double syy = m_[kYY];
      double inv_det = 1.0 / (sxx * syy - sxy * syx);
      if (std::isnan(inv_det)) {
        break;
      }
      double sxt = m_[kXT];
      double syt = m_[kYT];
      if (std::isnan(sxt) || std::isnan(syt)) {
        break;
      }
      DlScalar isxx = +syy * inv_det;
      DlScalar isxy = -sxy * inv_det;
      DlScalar isyy = +sxx * inv_det;
      DlScalar isyx = -syx * inv_det;
      DlScalar isxt = (sxy * syt - syy * sxt) * inv_det;
      DlScalar isyt = (syx * sxt - sxx * syt) * inv_det;
      return DlTransform::MakeAffine2D(isxx, isxy, isxt,  //
                                       isyx, isyy, isyt);
    }

    case Complexity::kUnknown:
      FML_DCHECK(complexity_ != Complexity::kUnknown);

    case Complexity::kAffine3D:
    case Complexity::kPerspectiveOnlyZ:
    case Complexity::kPerspectiveAll: {
      DlTransform inverse;
      DlScalar determinant = Invert4x4Matrix(m_, inverse.m_);
      if (DlScalar_IsFinite(determinant) && determinant != 0.0f) {
        inverse.complexity_ = Complexity::kUnknown;
        return inverse;
      }
      break;
    }
  }
  return {};
}

DlScalar DlTransform::Invert4x4Matrix(const DlScalar inMatrix[16],
                                      DlScalar outMatrix[16]) {
  double a00 = inMatrix[0];
  double a01 = inMatrix[1];
  double a02 = inMatrix[2];
  double a03 = inMatrix[3];
  double a10 = inMatrix[4];
  double a11 = inMatrix[5];
  double a12 = inMatrix[6];
  double a13 = inMatrix[7];
  double a20 = inMatrix[8];
  double a21 = inMatrix[9];
  double a22 = inMatrix[10];
  double a23 = inMatrix[11];
  double a30 = inMatrix[12];
  double a31 = inMatrix[13];
  double a32 = inMatrix[14];
  double a33 = inMatrix[15];

  double b00 = a00 * a11 - a01 * a10;
  double b01 = a00 * a12 - a02 * a10;
  double b02 = a00 * a13 - a03 * a10;
  double b03 = a01 * a12 - a02 * a11;
  double b04 = a01 * a13 - a03 * a11;
  double b05 = a02 * a13 - a03 * a12;
  double b06 = a20 * a31 - a21 * a30;
  double b07 = a20 * a32 - a22 * a30;
  double b08 = a20 * a33 - a23 * a30;
  double b09 = a21 * a32 - a22 * a31;
  double b10 = a21 * a33 - a23 * a31;
  double b11 = a22 * a33 - a23 * a32;

  // Calculate the determinant
  double determinant =
      b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
  double invdet = 1.0 / determinant;
  if (!DlScalar_IsFinite(invdet)) {
    return 0.0f;
  }
  if (outMatrix) {
    b00 *= invdet;
    b01 *= invdet;
    b02 *= invdet;
    b03 *= invdet;
    b04 *= invdet;
    b05 *= invdet;
    b06 *= invdet;
    b07 *= invdet;
    b08 *= invdet;
    b09 *= invdet;
    b10 *= invdet;
    b11 *= invdet;

    // clang-format off
    outMatrix[ 0] = a11 * b11 - a12 * b10 + a13 * b09;
    outMatrix[ 1] = a02 * b10 - a01 * b11 - a03 * b09;
    outMatrix[ 2] = a31 * b05 - a32 * b04 + a33 * b03;
    outMatrix[ 3] = a22 * b04 - a21 * b05 - a23 * b03;
    outMatrix[ 4] = a12 * b08 - a10 * b11 - a13 * b07;
    outMatrix[ 5] = a00 * b11 - a02 * b08 + a03 * b07;
    outMatrix[ 6] = a32 * b02 - a30 * b05 - a33 * b01;
    outMatrix[ 7] = a20 * b05 - a22 * b02 + a23 * b01;
    outMatrix[ 8] = a10 * b10 - a11 * b08 + a13 * b06;
    outMatrix[ 9] = a01 * b08 - a00 * b10 - a03 * b06;
    outMatrix[10] = a30 * b04 - a31 * b02 + a33 * b00;
    outMatrix[11] = a21 * b02 - a20 * b04 - a23 * b00;
    outMatrix[12] = a11 * b07 - a10 * b09 - a12 * b06;
    outMatrix[13] = a00 * b09 - a01 * b07 + a02 * b06;
    outMatrix[14] = a31 * b01 - a30 * b03 - a32 * b00;
    outMatrix[15] = a20 * b03 - a21 * b01 + a22 * b00;
    // clang-format on

    // If 1/det overflows to infinity (i.e. det is denormalized)
    // or any of the inverted matrix values is non-finite,
    // return zero to indicate a non-invertible matrix.
    if (!DlScalars_AreAllFinite(outMatrix, 16)) {
      determinant = 0.0f;
    }
  }
  return determinant;
}

std::optional<DlFVector> DlTransform::ComputeTransformedExpansion(
    DlScalar dx,
    DlScalar dy) const {
  if (!DlScalars_AreFinite(dx, dy) || dx < 0 || dy < 0) {
    return {};
  }
  if (!IsFinite() || complexity() >= Complexity::kPerspectiveAll) {
    return {};
  }

  // The x and y scalars would have been used to expand a local space
  // rectangle which is then transformed by ctm. In order to do the
  // expansion correctly, we should look at the relevant math. The
  // 4 corners will be moved outward by the following vectors:
  //     (UL,UR,LR,LL) = ((-x, -y), (+x, -y), (+x, +y), (-x, +y))
  // After applying the transform, each of these vectors could be
  // pointing in any direction so we need to examine each transformed
  // delta vector and how it affected the bounds.
  // Looking at just the affine 2x3 entries of the CTM we can delta
  // transform these corner offsets and get the following:
  //     UL = dCTM(-x, -y) = (- mXX*x - mXY*y, - mYX*x - mYY*y)
  //     UR = dCTM(+x, -y) = (  mXX*x - mXY*y,   mYX*x - mYY*y)
  //     LR = dCTM(+x, +y) = (  mXX*x + mXY*y,   mYX*x + mYY*y)
  //     LL = dCTM(-x, +y) = (- mXX*x + mXY*y, - mYX*x + mYY*y)
  // The X vectors are all some variation of adding or subtracting
  // the sum of mXX*x and mXY*y or their difference. Similarly the Y
  // vectors are +/- the associated sum/difference of mYX*x and mYY*y.
  // The largest displacements, both left/right or up/down, will
  // happen when the signs of the mXX/mXY/mYX/mYY matrix entries
  // coincide with the signs of the scalars, i.e. are all positive.
  DlScalar mXX = abs(m_[kXX]);
  DlScalar mXY = abs(m_[kXY]);
  DlScalar mYX = abs(m_[kYX]);
  DlScalar mYY = abs(m_[kYY]);
  return DlFVector(         //
      mXX * dx + mXY * dy,  //
      mYX * dx + mYY * dy   //
  );
}

DlTransform DlTransform::WithIntegerTranslation() const {
  if (!IsFinite()) {
    return *this;
  }
  switch (complexity()) {
    case Complexity::kUnknown:
      FML_DCHECK(complexity_ != Complexity::kUnknown);

    case Complexity::kIdentity:
    case Complexity::kAffine3D:
    case Complexity::kPerspectiveOnlyZ:
    case Complexity::kPerspectiveAll:
      return *this;

    case Complexity::kTranslate2D:
    case Complexity::kScaleTranslate2D:
    case Complexity::kAffine2D:
      break;
  }
  DlScalar xt_rounded = round(m_[kXT]);
  DlScalar yt_rounded = round(m_[kYT]);
  if (m_[kXT] == xt_rounded && m_[kYT] == yt_rounded) {
    return *this;
  }
  DlTransform ret(*this);
  ret.m_[kXT] = xt_rounded;
  ret.m_[kYT] = yt_rounded;
  // Rounding kXT and kYT cannot change the complexity.
  return ret;
}

DlTransform DlTransform::AsDeltaTransform() const {
  DlTransform ret(*this);
  ret.m_[kXT] = 0.0f;
  ret.m_[kYT] = 0.0f;
  ret.m_[kZT] = 0.0f;
  if (ret.complexity_ == Complexity::kTranslate2D) {
    ret.complexity_ = Complexity::kIdentity;
  }
  return ret;
}

std::ostream& operator<<(std::ostream& os, const DlTransform& t) {
  // clang-format off
  if (t.Is2D()) {
    return os << "DlAffine2D("
              << "[" << t.rc(0, 0) << ", " << t.rc(0, 1) << ", " << t.rc(0, 3) << "], "
              << "[" << t.rc(1, 0) << ", " << t.rc(1, 1) << ", " << t.rc(1, 3) << "]"
              << ")";
  }
  return os << "DlTransform<RowMajor>(["
      << "[" << t.rc(0, 0) << ", " << t.rc(0, 1) << ", " << t.rc(0, 2) << ", " << t.rc(0, 3) << "], "
      << "[" << t.rc(1, 0) << ", " << t.rc(1, 1) << ", " << t.rc(1, 2) << ", " << t.rc(1, 3) << "], "
      << "[" << t.rc(2, 0) << ", " << t.rc(2, 1) << ", " << t.rc(2, 2) << ", " << t.rc(2, 3) << "], "
      << "[" << t.rc(3, 0) << ", " << t.rc(3, 1) << ", " << t.rc(3, 2) << ", " << t.rc(3, 3) << "]"
      << ")";
  // clang-format on
}

}  // namespace flutter
