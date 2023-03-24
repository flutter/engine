// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "color.h"
#include "point.h"
#include "scalar.h"
#include "vector.h"

namespace impeller {

using Half = uint16_t;

/// @brief Convert a scalar to a half precision float.
///
/// Can express numbers in the range of 2^-14 to 65504.
/// Adapted from
/// https://developer.android.com/games/optimize/vertex-data-management .
Half ScalarToHalf(Scalar f);

template <class T, class U>
T Cast(const U& u);

template <>
Half Cast<Half, Scalar>(const Scalar& s);

template <>
Scalar Cast<Scalar, Scalar>(const Scalar& s);

struct HalfVector4 {
  union {
    struct {
      Half x = 0;
      Half y = 0;
      Half z = 0;
      Half w = 0;
    };
    Half e[4];
  };

  constexpr HalfVector4() {}

  constexpr HalfVector4(const Color& a)
      : x(ScalarToHalf(a.red)),
        y(ScalarToHalf(a.green)),
        z(ScalarToHalf(a.green)),
        w(ScalarToHalf(a.alpha)){};

  constexpr HalfVector4(const Vector4& a)
      : x(ScalarToHalf(a.x)),
        y(ScalarToHalf(a.y)),
        z(ScalarToHalf(a.z)),
        w(ScalarToHalf(a.w)){};
};

struct HalfVector3 {
  union {
    struct {
      Half x = 0;
      Half y = 0;
      Half z = 0;
    };
    Half e[3];
  };

  constexpr HalfVector3(){};

  constexpr HalfVector3(const Vector3& a)
      : x(ScalarToHalf(a.x)), y(ScalarToHalf(a.y)), z(ScalarToHalf(a.z)){};
};

struct HalfVector2 {
  union {
    struct {
      Half x = 0;
      Half y = 0;
    };
    Half e[2];
  };

  constexpr HalfVector2(){};

  constexpr HalfVector2(const Vector2& a)
      : x(ScalarToHalf(a.x)), y(ScalarToHalf(a.y)){};
};

}  // namespace impeller
