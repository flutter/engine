// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cmath>
#include <sstream>
#include <string>

#include "impeller/geometry/color.h"
#include "impeller/geometry/point.h"
#include "impeller/geometry/scalar.h"
#include "impeller/geometry/size.h"

namespace impeller {

template <class T, class U>
T Cast(const U& u);

template <>
Half Cast<Half, Scalar>(const Scalar& s);

template <>
Half Cast<Half, Half>(const Half& s);

template <>
Scalar Cast<Scalar, Scalar>(const Scalar& s);

template <class T>
struct TVector3 {
  using Type = T;

  union {
    struct {
      Type x = 0.0;
      Type y = 0.0;
      Type z = 0.0;
    };
    Type e[3];
  };

  constexpr TVector3(){};

  constexpr TVector3(const Color& c) : x(c.red), y(c.green), z(c.blue) {}

  constexpr TVector3(const Point& p) : x(p.x), y(p.y) {}

  constexpr TVector3(const Size& s) : x(s.width), y(s.height) {}

  constexpr TVector3(Type x, Type y) : x(x), y(y) {}

  constexpr TVector3(Type x, Type y, Type z) : x(x), y(y), z(z) {}

  template <class U>
  constexpr TVector3(const TVector3<U>& v)
      : x(Cast<T, U>(v.x)), y(Cast<T, U>(v.y)), z(Cast<T, U>(v.z)) {}

  /**
   *  The length (or magnitude of the vector).
   *
   *  @return the calculated length.
   */
  constexpr Scalar Length() const { return sqrt(x * x + y * y + z * z); }

  constexpr TVector3 Normalize() const {
    const auto len = Length();
    return {x / len, y / len, z / len};
  }

  template <class U>
  constexpr Scalar Dot(const TVector3<U>& other) const {
    return ((x * other.x) + (y * other.y) + (z * other.z));
  }

  template <class U>
  constexpr TVector3 Cross(const TVector3<U>& other) const {
    return {
        static_cast<Type>((y * other.z) - (z * other.y)),  //
        static_cast<Type>((z * other.x) - (x * other.z)),  //
        static_cast<Type>((x * other.y) - (y * other.x))   //
    };
  }

  constexpr TVector3 Min(const TVector3& p) const {
    return {std::min(x, p.x), std::min(y, p.y), std::min(z, p.z)};
  }

  constexpr TVector3 Max(const TVector3& p) const {
    return {std::max(x, p.x), std::max(y, p.y), std::max(z, p.z)};
  }

  constexpr TVector3 Floor() const {
    return {std::floor(x), std::floor(y), std::floor(z)};
  }

  constexpr TVector3 Ceil() const {
    return {std::ceil(x), std::ceil(y), std::ceil(z)};
  }

  constexpr TVector3 Round() const {
    return {std::round(x), std::round(y), std::round(z)};
  }

  template <class U>
  constexpr bool operator==(const TVector3<U>& v) const {
    return v.x == x && v.y == y && v.z == z;
  }

  template <class U>
  constexpr bool operator!=(const TVector3<U>& v) const {
    return v.x != x || v.y != y || v.z != z;
  }

  template <class U>
  constexpr TVector3 operator+=(const TVector3<U>& p) {
    x += static_cast<Type>(p.x);
    y += static_cast<Type>(p.y);
    z += static_cast<Type>(p.z);
    return *this;
  }

  template <class U>
  constexpr TVector3 operator-=(const TVector3<U>& p) {
    x -= static_cast<Type>(p.x);
    y -= static_cast<Type>(p.y);
    z -= static_cast<Type>(p.z);
    return *this;
  }

  template <class U>
  constexpr TVector3 operator*=(const TVector3<U>& p) {
    x *= static_cast<Type>(p.x);
    y *= static_cast<Type>(p.y);
    z *= static_cast<Type>(p.z);
    return *this;
  }

  template <class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr TVector3 operator*=(U scale) {
    x *= scale;
    y *= scale;
    z *= scale;
    return *this;
  }

  template <class U>
  constexpr TVector3 operator/=(const TVector3<U>& p) {
    x /= static_cast<Type>(p.x);
    y /= static_cast<Type>(p.y);
    z /= static_cast<Type>(p.z);
    return *this;
  }

  template <class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr TVector3 operator/=(U scale) {
    x /= scale;
    y /= scale;
    z /= scale;
    return *this;
  }

  constexpr TVector3 operator-() const { return TVector3(-x, -y, -z); }

  template <class U>
  constexpr TVector3 operator+(const TVector3<U>& v) const {
    return TVector3(x + static_cast<Type>(v.x), y + static_cast<Type>(v.y),
                    z + static_cast<Type>(v.z));
  }

  template <class U>
  constexpr TVector3 operator-(const TVector3<U>& v) const {
    return TVector3(x - static_cast<Type>(v.x), y - static_cast<Type>(v.y),
                    z - static_cast<Type>(v.z));
  }

  template <class U>
  constexpr TVector3 operator*(const TVector3<U>& v) const {
    return TVector3(x * static_cast<Type>(v.x), y * static_cast<Type>(v.y),
                    z * static_cast<Type>(v.z));
  }

  template <class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr TVector3 operator*(U scale) const {
    return TVector3(x * static_cast<Type>(scale), y * static_cast<Type>(scale),
                    z * static_cast<Type>(scale));
  }

  template <class U>
  constexpr TVector3 operator/(const TVector3<U>& v) const {
    return TVector3(x / static_cast<Type>(v.x), y / static_cast<Type>(v.y),
                    z / static_cast<Type>(v.z));
  }

  template <class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr TVector3 operator/(U scale) const {
    return TVector3(x / static_cast<Type>(scale), y / static_cast<Type>(scale),
                    z / static_cast<Type>(scale));
  }

  constexpr TVector3 Lerp(const TVector3& v, Scalar t) const {
    return *this + (v - *this) * t;
  }

  /**
   *  Make a linear combination of two vectors and return the result.
   *
   *  @param a      the first vector.
   *  @param aScale the scale to use for the first vector.
   *  @param b      the second vector.
   *  @param bScale the scale to use for the second vector.
   *
   *  @return the combined vector.
   */
  template <class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
  static constexpr TVector3<U> Combine(const TVector3<U>& a,
                                       Scalar aScale,
                                       const TVector3<U>& b,
                                       Scalar bScale) {
    return {
        aScale * a.x + bScale * b.x,  //
        aScale * a.y + bScale * b.y,  //
        aScale * a.z + bScale * b.z,  //
    };
  }

  std::string ToString() const {
    std::stringstream stream;
    stream << "{" << x << ", " << y << ", " << z << "}";
    return stream.str();
  }
};

// RHS algebraic operations with arithmetic types.

template <class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr TVector3<U> operator*(U s, const TVector3<U>& p) {
  return p * s;
}

template <class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
constexpr TVector3<U> operator/(U s, const TVector3<U>& p) {
  return {
      s / p.x,
      s / p.y,
      s / p.z,
  };
}

template <class T>
struct TVector4 {
  using Type = T;
  union {
    struct {
      Type x = 0.0;
      Type y = 0.0;
      Type z = 0.0;
      Type w = 1.0;
    };
    Type e[4];
  };

  constexpr TVector4() {}

  template <class U = Scalar>
  constexpr TVector4(const Color& c)
      : x(Cast<T, U>(c.red)),
        y(Cast<T, U>(c.green)),
        z(Cast<T, U>(c.blue)),
        w(Cast<T, U>(c.alpha)) {}

  constexpr TVector4(Type x, Type y, Type z, Type w) : x(x), y(y), z(z), w(w) {}

  template <class U>
  constexpr TVector4(const TVector4<U>& v)
      : x(Cast<T, U>(v.x)),
        y(Cast<T, U>(v.y)),
        z(Cast<T, U>(v.z)),
        w(Cast<T, U>(v.w)) {}

  template <class U>
  constexpr TVector4(const TVector3<U>& v)
      : x(Cast<T, U>(v.x)),
        y(Cast<T, U>(v.y)),
        z(Cast<T, U>(v.z)) {}

  constexpr TVector4(const Point& p) : x(p.x), y(p.y) {}

  TVector4 Normalize() const {
    const Scalar inverse = 1.0 / sqrt(x * x + y * y + z * z + w * w);
    return TVector4(x * inverse, y * inverse, z * inverse, w * inverse);
  }

  template <class U>
  constexpr bool operator==(const TVector4<U>& v) const {
    return (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w);
  }

  template <class U>
  constexpr bool operator!=(const TVector4<U>& v) const {
    return (x != v.x) || (y != v.y) || (z != v.z) || (w != v.w);
  }

  template <class U>
  constexpr TVector4 operator+(const TVector4<U>& v) const {
    return TVector4(x + v.x, y + v.y, z + v.z, w + v.w);
  }

  template <class U>
  constexpr TVector4 operator-(const TVector4<U>& v) const {
    return TVector4(x - v.x, y - v.y, z - v.z, w - v.w);
  }

  template <class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr TVector4 operator*(U f) const {
    return TVector4(x * f, y * f, z * f, w * f);
  }

  template <class U>
  constexpr TVector4 operator*(const TVector4<U>& v) const {
    return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
  }

  constexpr TVector4 Min(const TVector4& p) const {
    return {std::min(x, p.x), std::min(y, p.y), std::min(z, p.z),
            std::min(w, p.w)};
  }

  constexpr TVector4 Max(const TVector4& p) const {
    return {std::max(x, p.x), std::max(y, p.y), std::max(z, p.z),
            std::max(w, p.w)};
  }

  constexpr TVector4 Floor() const {
    return {std::floor(x), std::floor(y), std::floor(z), std::floor(w)};
  }

  constexpr TVector4 Ceil() const {
    return {std::ceil(x), std::ceil(y), std::ceil(z), std::ceil(w)};
  }

  constexpr TVector4 Round() const {
    return {std::round(x), std::round(y), std::round(z), std::round(w)};
  }

  constexpr TVector4 Lerp(const TVector4& v, Scalar t) const {
    return *this + (v - *this) * t;
  }

  std::string ToString() const {
    std::stringstream stream;
    stream << "{" << x << ", " << y << ", " << z << ", " << w << "}";
    return stream.str();
  }
};

using Vector3 = TVector3<Scalar>;
using HalfVector3 = TVector3<Half>;
using Vector4 = TVector4<Scalar>;
using HalfVector4 = TVector4<Half>;

static_assert(sizeof(Vector3) == 3 * sizeof(Scalar));
static_assert(sizeof(Vector4) == 4 * sizeof(Scalar));
static_assert(sizeof(HalfVector4) == 4 * sizeof(Half));

}  // namespace impeller

namespace std {

inline std::ostream& operator<<(std::ostream& out, const impeller::Vector3& p) {
  out << "(" << p.x << ", " << p.y << ", " << p.z << ")";
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const impeller::Vector4& p) {
  out << "(" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")";
  return out;
}

inline std::ostream& operator<<(std::ostream& out,
                                const impeller::HalfVector4& p) {
  out << "(" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")";
  return out;
}

}  // namespace std
