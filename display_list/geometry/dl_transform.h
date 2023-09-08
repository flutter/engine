// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_TRANSFORM_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_TRANSFORM_H_

#include <algorithm>
#include <limits>
#include <optional>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/geometry/dl_angle.h"
#include "flutter/display_list/geometry/dl_homogenous.h"
#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_rect.h"

#include "third_party/skia/include/core/SkMatrix.h"

namespace flutter {

class DlTransform {
 public:
  // clang-format off
  DlTransform() : DlTransform(
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
      Complexity::kIdentity) {}
  // clang-format on

  static DlTransform MakeTranslate(DlScalar tx, DlScalar ty) {
    // clang-format off
    return {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
          tx,   ty, 0.0f, 1.0f,
        Complexity::kTranslate2D
    };
    // clang-format on
  }
  static DlTransform MakeTranslate(DlFPoint p) {
    return MakeTranslate(p.x(), p.y());
  }

  static DlTransform MakeScale(DlScalar sx, DlScalar sy) {
    // clang-format off
    return {
          sx, 0.0f, 0.0f, 0.0f,
        0.0f,   sy, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        Complexity::kScaleTranslate2D
    };
    // clang-format on
  }

  static DlTransform MakeAnchoredScale(DlScalar sx,
                                       DlScalar sy,
                                       DlScalar tx,
                                       DlScalar ty) {
    tx = tx - tx * sx;
    ty = ty - ty * sy;
    // clang-format off
    return {
          sx, 0.0f, 0.0f, 0.0f,
        0.0f,   sy, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
          tx,   ty, 0.0f, 1.0f,
        Complexity::kScaleTranslate2D
    };
    // clang-format on
  }

  static DlTransform MakeSkew(DlScalar sx, DlScalar sy) {
    // clang-format off
    return {
        1.0f,   sy, 0.0f, 0.0f,
          sx, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        Complexity::kAffine2D
    };
    // clang-format on
  }

  static DlTransform MakeCosSin(DlScalar cos, DlScalar sin) {
    // clang-format off
    return {
         cos,  sin, 0.0f, 0.0f,
        -sin,  cos, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        Complexity::kAffine2D
    };
    // clang-format on
  }
  static DlTransform MakeCosSin(const DlFVector& cos_sin) {
    return MakeCosSin(cos_sin.x(), cos_sin.y());
  }
  static DlTransform MakeRotate(DlAngle angle) {
    return MakeCosSin(angle.CosSin());
  }
  static DlTransform MakeCosSin(DlFVector3 axis, DlScalar cos, DlScalar sin) {
    // Copied from Skia, which in turn:
    // Taken from "Essential Mathematics for Games and Interactive Applications"
    //             James M. Van Verth and Lars M. Bishop -- third edition
    DlScalar len = axis.length();
    if (!DlScalar_IsFinite(len) || DlScalar_IsNearlyZero(len)) {
      return DlTransform();
    }
    axis = axis / len;
    DlScalar x = axis.x();
    DlScalar y = axis.y();
    DlScalar z = axis.z();
    DlScalar c = cos;
    DlScalar s = sin;
    DlScalar t = 1 - c;

    // Note that this is transposed as compared to the version in the Skia
    // sources because, while both of our internal matrices may be col major,
    // their constructor is row major and so their initializer list was
    // transposed compared to the col major format of our matrix array.
    // clang-format off
    return {
        t*x*x + c,   t*x*y + s*z, t*x*z - s*y, 0,
        t*x*y - s*z, t*y*y + c,   t*y*z + s*x, 0,
        t*x*z + s*y, t*y*z - s*x, t*z*z + c,   0,
        0,           0,           0,           1,
        Complexity::kUnknown
    };
    // clang-format on
  }
  static DlTransform MakeCosSin(DlFVector3 axis, DlFVector cos_sin) {
    return MakeCosSin(axis, cos_sin.x(), cos_sin.y());
  }
  static DlTransform MakeRotate(DlFVector3 axis, DlAngle angle) {
    return MakeCosSin(axis, angle.CosSin());
  }

  // clang-format off
  static DlTransform MakeAffine2D(DlScalar mxx, DlScalar mxy, DlScalar mxt,
                                  DlScalar myx, DlScalar myy, DlScalar myt) {
    return {
         mxx,  myx, 0.0f, 0.0f,
         mxy,  myy, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
         mxt,  myt, 0.0f, 1.0f,
        Complexity::kAffine2D
    };
  }
  // clang-format on

  // Constructs a DlTransform from 16 matrix coefficients in row major format.
  // In other words, to compute the transformed X coordinate one would
  // use the formula |x * mxx + y * mxy + z * mxz + mxt|.
  // clang-format off
  static DlTransform MakeRowMajor(
      DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
      DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
      DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
      DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) {
    return {
        mxx, myx, mzx, mwx,
        mxy, myy, mzy, mwy,
        mxz, myz, mzz, mwz,
        mxt, myt, mzt, mwt,
        Complexity::kUnknown
    };
  }
  // clang-format on

  // Constructs a DlTransform from 16 matrix coefficients in column major
  // format.
  // In other words, to compute the transformed X coordinate one would
  // use the formula |x * mxx + y * mxy + z * mxz + mxt|.
  // clang-format off
  static DlTransform MakeColMajor(
      DlScalar mxx, DlScalar myx, DlScalar mzx, DlScalar mwx,
      DlScalar mxy, DlScalar myy, DlScalar mzy, DlScalar mwy,
      DlScalar mxz, DlScalar myz, DlScalar mzz, DlScalar mwz,
      DlScalar mxt, DlScalar myt, DlScalar mzt, DlScalar mwt) {
    return {
        mxx, myx, mzx, mwx,
        mxy, myy, mzy, mwy,
        mxz, myz, mzz, mwz,
        mxt, myt, mzt, mwt,
        Complexity::kUnknown
    };
  }
  // clang-format on

  // Constructs a DlTransform from an array containing 16 matrix coefficients
  // in column major format.
  // In other words, to compute the transformed X coordinate one would
  // use the formula |x * m[0] + y * m[4] + z * m[8] + m[12]|.
  static DlTransform MakeColMajor(DlScalar m[16]) {
    // clang-format off
    return {
        m[ 0], m[ 1], m[ 2], m[ 3],
        m[ 4], m[ 5], m[ 6], m[ 7],
        m[ 8], m[ 9], m[10], m[11],
        m[12], m[13], m[14], m[15],
        Complexity::kUnknown
    };
    // clang-format on
  }

  // Constructs a DlTransform from an array containing 16 matrix coefficients
  // in row major format.
  // In other words, to compute the transformed X coordinate one would
  // use the formula |x * m[0] + y * m[1] + z * m[2] + m[3]|.
  static DlTransform MakeRowMajor(DlScalar m[16]) {
    // clang-format off
    return {
        m[ 0], m[ 4], m[ 8], m[12],
        m[ 1], m[ 5], m[ 9], m[13],
        m[ 2], m[ 6], m[10], m[14],
        m[ 3], m[ 7], m[11], m[15],
        Complexity::kUnknown
    };
    // clang-format on
  }

  static DlTransform MakeConcat(const DlTransform& outer,
                                const DlTransform& inner);

  // Retrieves the 16 matrix coefficients for the transform in column major
  // format.
  // From the result, to compute the transformed X coordinate one would
  // use the formula |x * m[0] + y * m[4] + z * m[8] + m[12]|.
  void GetColMajor(DlScalar matrix[16]) const {
    static_assert(sizeof(m_) == 64);
    memcpy(matrix, m_, sizeof(m_));
  }

  // Retrieves the 16 matrix coefficients for the transform in row major
  // format.
  // From the result, to compute the transformed X coordinate one would
  // use the formula |x * m[0] + y * m[1] + z * m[2] + m[3]|.
  void GetRowMajor(DlScalar matrix[16]) const {
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        matrix[r * 4 + c] = m_[c * 4 + r];
      }
    }
  }

  void SetIdentity() {
    InitIdentity();
    complexity_ = Complexity::kIdentity;
  }

  void SetTranslate(DlScalar tx, DlScalar ty) {
    InitIdentity();
    m_[kXT] = tx;
    m_[kYT] = ty;
    complexity_ = Complexity::kTranslate2D;
  }

  void SetScale(DlScalar sx, DlScalar sy) {
    InitIdentity();
    m_[kXX] = sx;
    m_[kYY] = sy;
    complexity_ = Complexity::kScaleTranslate2D;
  }

  void SetAnchoredScale(DlScalar sx, DlScalar sy, DlScalar tx, DlScalar ty) {
    InitIdentity();
    m_[kXX] = sx;
    m_[kYY] = sy;
    m_[kXT] = tx - sx * tx;
    m_[kYT] = ty - sy * ty;
    complexity_ = Complexity::kScaleTranslate2D;
  }

  void SetCosSin(DlFVector cos_sin) {
    InitIdentity();
    m_[kXX] = +cos_sin.x();
    m_[kXY] = -cos_sin.y();
    m_[kYX] = +cos_sin.y();
    m_[kYY] = +cos_sin.x();
    complexity_ = Complexity::kAffine2D;
  }
  void SetRotate(DlAngle angle) { SetCosSin(angle.CosSin()); }
  void SetRotate(const DlFVector3& axis, DlAngle angle) {
    // Copied from Skia, which in turn:
    // Taken from "Essential Mathematics for Games and Interactive Applications"
    //             James M. Van Verth and Lars M. Bishop -- third edition
    DlScalar len = axis.length();
    if (!DlScalar_IsFinite(len) || DlScalar_IsNearlyZero(len)) {
      SetIdentity();
      return;
    }
    const DlFVector3 unit_axis = axis / len;
    const DlFVector cos_sin = angle.CosSin();
    DlScalar x = unit_axis.x();
    DlScalar y = unit_axis.y();
    DlScalar z = unit_axis.z();
    DlScalar c = cos_sin.x();
    DlScalar s = cos_sin.y();
    DlScalar t = 1 - c;

    // Note that this is transposed as compared to the version in the Skia
    // sources because, while both of our internal matrices may be col major,
    // their constructor is row major and so their initializer list was
    // transposed compared to the col major format of our matrix array.
    // clang-format off
    init_matrix(m_,
        t*x*x + c,   t*x*y + s*z, t*x*z - s*y, 0,
        t*x*y - s*z, t*y*y + c,   t*y*z + s*x, 0,
        t*x*z + s*y, t*y*z - s*x, t*z*z + c,   0,
        0,           0,           0,           1
    );
    // clang-format on
    complexity_ = Complexity::kUnknown;
  }

  DlTransform& TranslateInner(DlScalar tx, DlScalar ty);
  DlTransform& TranslateOuter(DlScalar tx, DlScalar ty);
  DlTransform& TranslateInner(DlFPoint p) {
    return TranslateInner(p.x(), p.y());
  }
  DlTransform& TranslateOuter(DlFPoint p) {
    return TranslateOuter(p.x(), p.y());
  }

  DlTransform& ScaleInner(DlScalar sx, DlScalar sy);
  DlTransform& ScaleOuter(DlScalar sx, DlScalar sy);

  DlTransform& SkewInner(DlScalar sx, DlScalar sy);
  DlTransform& SkewOuter(DlScalar sx, DlScalar sy);

  DlTransform& RotateInner(DlAngle angle);
  DlTransform& RotateOuter(DlAngle angle);

  DlTransform& RotateInner(DlFVector3 axis, DlAngle angle);
  DlTransform& RotateOuter(DlFVector3 axis, DlAngle angle);

  DlTransform& ConcatInner(const DlTransform& inner);
  DlTransform& ConcatOuter(const DlTransform& outer);

  bool operator==(const DlTransform& other) const {
    static_assert(sizeof(m_) == 64);
    return memcmp(m_, other.m_, sizeof(m_)) == 0;
  }

  bool operator!=(const DlTransform& other) const { return !(*this == other); }

  bool IsIdentity() const { return complexity() <= Complexity::kIdentity; }
  bool IsTranslate() const { return complexity() <= Complexity::kTranslate2D; }
  bool IsScaleTranslate() const {
    return complexity() <= Complexity::kScaleTranslate2D;
  }
  bool Is2D() const { return complexity() <= Complexity::kAffine2D; }
  bool HasPerspective() const { return complexity() > Complexity::kAffine3D; }
  bool IsFinite() const { return DlScalars_AreAllFinite(m_, 16); }
  bool RectStaysRect() const;

  DlFPoint TransformPoint(DlScalar x, DlScalar y) const;
  DlFPoint TransformPoint(const DlFPoint& point) const {
    return TransformPoint(point.x(), point.y());
  }
  DlFPoint TransformPoint(const DlIPoint& point) const {
    return TransformPoint(point.x(), point.y());
  }

  void TransformPoints(DlFPoint dst[], const DlFPoint src[], int count) const;

  DlFHomogenous3D TransformHomogenous(DlScalar x,
                                      DlScalar y,
                                      DlScalar z = 0.0f,
                                      DlScalar w = 1.0f) const;
  DlFHomogenous3D TransformHomogenous(const DlFPoint& point) const {
    return TransformHomogenous(point.x(), point.y());
  }
  DlFHomogenous3D TransformHomogenous(const DlFHomogenous3D& homogenous) const {
    return TransformHomogenous(homogenous.x(), homogenous.y(), homogenous.z(),
                               homogenous.w());
  }

  DlFRect TransformRect(const DlFRect& rect) const;
  DlFRect TransformRect(const DlIRect& rect) const {
    return TransformRect(DlFRect::MakeBounds(rect));
  }

  // Tries to compute the total X & Y expansion factors that would be
  // applied in device space to the transformed bounds of a rectangle
  // as if the indicated delta values had been applied to the rectangle
  // before it was transformed.
  // Returns no answer if any of the conditions hold:
  // - dx, dy are non-finite or negative
  // - matrix is non-finite or has perspective components
  std::optional<DlFVector> ComputeTransformedExpansion(DlScalar dx,
                                                       DlScalar dy) const;

  DlScalar Determinant() const;
  bool IsInvertible() const {
    DlScalar d = Determinant();
    return DlScalar_IsFinite(d) && d != 0.0;
  }

  std::optional<DlTransform> Inverse() const;

  // Produces a version of this transform with integer translation
  // components.
  DlTransform WithIntegerTranslation() const;

  // Produces a version of this tranfsorm with zeroed translation components
  DlTransform AsDeltaTransform() const;

  // Return the indicated entry from the transform matrix as if it was
  // implemented as a row-major matrix.
  DlScalar rc(int row, int col) const { return m_[col * 4 + row]; }

  // Return the indicated entry from the transform matrix as if it was
  // implemented as a col-major matrix.
  DlScalar cr(int row, int col) const { return m_[row * 4 + col]; }

  DlFVector4 eqnX() const { return {m_[kXX], m_[kXY], m_[kXZ], m_[kXT]}; }
  DlFVector4 eqnY() const { return {m_[kYX], m_[kYY], m_[kYZ], m_[kYT]}; }
  DlFVector4 eqnZ() const { return {m_[kZX], m_[kZY], m_[kZZ], m_[kZT]}; }
  DlFVector4 eqnW() const { return {m_[kWX], m_[kWY], m_[kWZ], m_[kWT]}; }

  DlFVector3 eqnX_3() const { return {m_[kXX], m_[kXY], m_[kXT]}; }
  DlFVector3 eqnY_3() const { return {m_[kYX], m_[kYY], m_[kYT]}; }
  DlFVector3 eqnW_3() const { return {m_[kWX], m_[kWY], m_[kWT]}; }

  SkMatrix ToSkMatrix() const {
    return SkMatrix::MakeAll(       //
        m_[kXX], m_[kXY], m_[kXT],  //
        m_[kYX], m_[kYY], m_[kYT],  //
        m_[kWX], m_[kWY], m_[kWT]   //
    );
  };

 private:
  // kPQ is the index of a coefficient in the equation to produce P
  // all four Q factors are multiplied by the associated value in the
  // source coordinate (X,Y,Z,1) or (X,Y,0,1) and summed to produce
  // the resulting P value
  // Note that the k_T values are all multiplied by a 1 in the source
  // coordinate because they are just the translation value
  static constexpr int kXX = 0;
  static constexpr int kXY = 4;
  static constexpr int kXZ = 8;
  static constexpr int kXT = 12;

  static constexpr int kYX = 1;
  static constexpr int kYY = 5;
  static constexpr int kYZ = 9;
  static constexpr int kYT = 13;

  static constexpr int kZX = 2;
  static constexpr int kZY = 6;
  static constexpr int kZZ = 10;
  static constexpr int kZT = 14;

  static constexpr int kWX = 3;
  static constexpr int kWY = 7;
  static constexpr int kWZ = 11;
  static constexpr int kWT = 15;

  enum class Complexity {
    // Matrix is identity
    kIdentity,

    // Matrix contains only X and Y translate components
    kTranslate2D,

    // Matrix contains only X and Y scale and translate components
    // There are no mixing components to cause Y to impact the X result
    // or vice versa.
    kScaleTranslate2D,

    // Matrix contains only values that operate on the incoming X and Y
    // coordinates plus translation for X and Y
    kAffine2D,

    // Matrix contains values that operate on X, Y, and Z input coordinates
    // plus translations for those coordinates.
    // Affine3D can be treated as Affine2D for 2D -> 2D transformations
    // The coordinates in those operations are of the form [x, y, 0, 1]
    // and so they don't even process the Z coefficients, and the result
    // contains only an X and a Y coordinate so the result of the entire
    // Z equation is ignored
    kAffine3D,

    // PerspectiveOnlyZ means that only a non-zero Z coordinate will
    // induce perspective by making the W result component non-unity.
    // (The W result equation is |0*x + 0*y + N*z + 1|.)
    // Since most operations are 2D only with coordinates of the form
    // [x, y, 0, 1], this type of matrix can be treated like Affine3D
    // or Affine2D as appropriate.
    kPerspectiveOnlyZ,

    // PerspectiveAll means that any 2D or 3D coordinate can result
    // in a non-unity W result.
    kPerspectiveAll,

    // Safe value that causes the complexity to be reevaluated the next
    // time it is needed.
    kUnknown,

    kLast = kUnknown,
  };

  static constexpr DlScalar identity_values[16] = {
      // clang-format off
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
      // clang-format on
  };
  void InitIdentity() { memcpy(m_, identity_values, sizeof(m_)); }

  static constexpr Complexity complexity_values[16] = {
      // clang-format off
      Complexity::kScaleTranslate2D,
      Complexity::kAffine2D,
      Complexity::kAffine3D,
      Complexity::kPerspectiveAll,

      Complexity::kAffine2D,
      Complexity::kScaleTranslate2D,
      Complexity::kAffine3D,
      Complexity::kPerspectiveAll,

      Complexity::kAffine3D,
      Complexity::kAffine3D,
      Complexity::kAffine3D,
      Complexity::kPerspectiveOnlyZ,

      Complexity::kTranslate2D,
      Complexity::kTranslate2D,
      Complexity::kAffine3D,
      Complexity::kPerspectiveAll,
      // clang-format on
  };

  static Complexity ComputeComplexity(const DlScalar matrix[16]);
  inline Complexity complexity() const {
    if (complexity_ == Complexity::kUnknown) {
      complexity_ = ComputeComplexity(m_);
    }
    return complexity_;
  }

  static DlScalar Invert4x4Matrix(const DlScalar inMatrix[16],
                                  DlScalar outMatrix[16]);

  // The 2 init_matrix templates allow for a compact initialization of the
  // already allocated 16 entries in the array whether via constructor or
  // a post-constructor setter
  template <typename T, typename U>
  void init_matrix(T* array, U x) {
    *array = x;
  }

  template <typename T, typename U, typename... V>
  void init_matrix(T* array, U x, V... y) {
    *array = x;
    init_matrix(array + 1, y...);
  }

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
  static_assert(sizeof(m_) == sizeof(identity_values));
  static_assert(sizeof(m_) == sizeof(DlScalar) * 16);
};

extern std::ostream& operator<<(std::ostream& os, const DlTransform& t);

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_TRANSFORM_H_
