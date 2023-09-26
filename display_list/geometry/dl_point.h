// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_POINT_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_POINT_H_

#include <algorithm>
#include <limits>
#include <ostream>

#include "flutter/display_list/dl_base_types.h"

namespace flutter {

#define DL_ONLY_ON_FLOAT(Type) DL_ONLY_ON_FLOAT_M(, Type)
#define DL_ONLY_ON_FLOAT_M(Modifiers, Type) \
  template <typename U = T>                 \
  Modifiers std::enable_if_t<std::is_floating_point_v<U>, Type>

template <typename T>
struct DlTPoint {
 private:
  friend struct DlFVector3;
  friend struct DlFVector4;

  static constexpr T zero_ = static_cast<T>(0);

  T x_;
  T y_;

 public:
  constexpr DlTPoint() : DlTPoint(zero_, zero_) {}
  constexpr DlTPoint(T x, T y) : x_(x), y_(y) {}

  template <typename U>
  constexpr explicit DlTPoint(const U& p) {
    DlTpoint(p.x(), p.y());
  }

  constexpr inline T x() const { return x_; }
  constexpr inline T y() const { return y_; }

  constexpr inline T Length() const { return std::sqrt(x_ * x_ + y_ * y_); }

  template <typename U>
  void operator=(const U& p) {
    x_ = p.x();
    y_ = p.y();
  }

  DlTPoint operator+(const DlTPoint& p) const { return {x_ + p.x_, y_ + p.y_}; }
  DlTPoint& operator+=(const DlTPoint& p) {
    *this = *this + p;
    return *this;
  }
  DlTPoint operator-(const DlTPoint& p) const { return {x_ - p.x_, y_ - p.y_}; }
  DlTPoint& operator-=(const DlTPoint& p) {
    *this = *this - p;
    return *this;
  }
  DlTPoint operator-() const { return {-x_, -y_}; }

  bool operator==(const DlTPoint& p) const { return x_ == p.x_ && y_ == p.y_; }
  bool operator!=(const DlTPoint& p) const { return !(*this == p); }

  DL_ONLY_ON_FLOAT(bool)
  IsFinite() const { return DlScalars_AreFinite(x_, y_); }
};

using DlFPoint = DlTPoint<DlScalar>;
using DlIPoint = DlTPoint<DlInt>;
using DlFVector = DlFPoint;
using DlIVector = DlIPoint;

static inline DlIVector Floor(const DlFVector& vector) {
  return DlIPoint(DlScalar_ToInt(floor(vector.x())),
                  DlScalar_ToInt(floor(vector.y())));
}

static inline DlIVector Ceil(const DlFVector& vector) {
  return DlIPoint(DlScalar_ToInt(ceil(vector.x())),
                  DlScalar_ToInt(ceil(vector.y())));
}

template <typename T>
inline DlTPoint<T> operator*(const DlTPoint<T>& p, T v) {
  return {p.x() * v, p.y() * v};
}
template <typename T>
inline DlTPoint<T> operator*(T v, const DlTPoint<T>& p) {
  return p * v;
}
template <typename T>
inline DlTPoint<T> operator/(const DlTPoint<T>& p, T v) {
  return p * (1.0f / v);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const DlTPoint<T>& point) {
  return os << "DlPoint(" << point.x() << ", " << point.y() << ")";
}

struct DlFVector3 : public DlTPoint<DlScalar> {
 protected:
  friend struct DlFVector4;

  DlScalar z_;

 public:
  constexpr DlFVector3() : DlFVector3(0.0f, 0.0f, 0.0f) {}
  constexpr DlFVector3(DlScalar x, DlScalar y, DlScalar z)
      : DlTPoint(x, y), z_(z) {}

  template <typename U>
  explicit constexpr DlFVector3(const U& p) {
    DlFVector3(p.x(), p.y(), p.z());
  }

  constexpr inline DlScalar z() const { return z_; }
  constexpr DlScalar length() const {
    return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
  }

  constexpr inline DlScalar operator[](int i) const {
    return static_cast<const DlScalar*>(&x_)[i];
  }

  template <typename U>
  void operator=(const U& p) {
    x_ = p.x();
    y_ = p.y();
    z_ = p.z();
  }

  DlFVector3 operator+(const DlFVector3& p) const {
    return {x_ + p.x_, y_ + p.y_, z_ + p.z_};
  }
  DlFVector3& operator+=(const DlFVector3& p) {
    *this = *this + p;
    return *this;
  }
  DlFVector3 operator-(const DlFVector3& p) const {
    return {x_ - p.x_, y_ - p.y_, z_ - p.z_};
  }
  DlFVector3& operator-=(const DlFVector3& p) {
    *this = *this - p;
    return *this;
  }
  DlFVector3 operator-() const { return {-x_, -y_, -z_}; }

  bool operator==(const DlFVector3& p) const {
    return x_ == p.x_ && y_ == p.y_ && z_ == p.z_;
  }
  bool operator!=(const DlFVector3& p) const { return !(*this == p); }

  bool IsFinite() const {
    return DlTPoint<DlScalar>::IsFinite() && DlScalar_IsFinite(z_);
  }
};

template <typename T>
inline DlFVector3 operator*(const DlFVector3 p, T v) {
  return {p.x() * v, p.y() * v, p.z() * v};
}
template <typename T>
inline DlFVector3 operator*(T v, const DlFVector3 p) {
  return p * v;
}
template <typename T>
inline DlFVector3 operator/(const DlFVector3 p, T v) {
  return p * (1.0f / v);
}

static inline std::ostream& operator<<(std::ostream& os,
                                       const DlFVector3& point) {
  return os << "DlVector3(" << point.x() << ", " << point.y() << ", "
            << point.z() << ")";
}

[[maybe_unused]] constexpr DlFVector3 kDlAxis_X = DlFVector3(1.0f, 0.0f, 0.0f);
[[maybe_unused]] constexpr DlFVector3 kDlAxis_Y = DlFVector3(0.0f, 1.0f, 0.0f);
[[maybe_unused]] constexpr DlFVector3 kDlAxis_Z = DlFVector3(0.0f, 0.0f, 1.0f);

struct DlFVector4 : public DlFVector3 {
 private:
  DlScalar w_;

 public:
  constexpr DlFVector4() : DlFVector4(0.0f, 0.0f, 0.0f, 1.0f) {}
  constexpr DlFVector4(DlScalar x, DlScalar y, DlScalar z, DlScalar w)
      : DlFVector3(x, y, z), w_(w) {}

  template <typename U>
  explicit constexpr DlFVector4(const U& p) {
    DlFVector3(p.x(), p.y(), p.z(), p.w());
  }

  constexpr inline DlScalar w() const { return w_; }
  constexpr DlScalar length() const {
    return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_) / w_;
  }

  constexpr inline DlScalar operator[](int i) const {
    return static_cast<const DlScalar*>(&x_)[i];
  }

  template <typename U>
  void operator=(const U& p) {
    x_ = p.x();
    y_ = p.y();
    z_ = p.z();
    w_ = p.w();
  }

  bool operator==(const DlFVector4& p) const {
    return x_ == p.x_ && y_ == p.y_ && z_ == p.z_ && w_ == p.w_;
  }
  bool operator!=(const DlFVector4& p) const { return !(*this == p); }

  bool IsFinite() const {
    return DlFVector3::IsFinite() && DlScalar_IsFinite(w_);
  }
};

#undef DL_ONLY_ON_FLOAT
#undef DL_ONLY_ON_FLOAT_M

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_POINT_H_
