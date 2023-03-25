// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>

#include "color.h"
#include "point.h"
#include "scalar.h"
#include "vector.h"

namespace impeller {

/// @brief Convert a scalar to a half precision float.
///
/// See also: https://clang.llvm.org/docs/LanguageExtensions.html
_Float16 ScalarToHalf(Scalar f);

/// @brief A storage only class for half precision floating point.
struct Half {
  _Float16 x = 0;

  Half() = default;

  Half(Scalar value) : x(ScalarToHalf(value)) {}
};

/// @brief A storage only class for half precision floating point vector 4.
struct HalfVector4 {
  union {
    struct {
      _Float16 x = 0;
      _Float16 y = 0;
      _Float16 z = 0;
      _Float16 w = 0;
    };
    _Float16 e[4];
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

/// @brief A storage only class for half precision floating point vector 3.
struct HalfVector3 {
  union {
    struct {
      _Float16 x = 0;
      _Float16 y = 0;
      _Float16 z = 0;
    };
    _Float16 e[3];
  };

  constexpr HalfVector3(){};

  constexpr HalfVector3(const Vector3& a)
      : x(ScalarToHalf(a.x)), y(ScalarToHalf(a.y)), z(ScalarToHalf(a.z)){};
};

/// @brief A storage only class for half precision floating point vector 2.
struct HalfVector2 {
  union {
    struct {
      _Float16 x = 0;
      _Float16 y = 0;
    };
    _Float16 e[2];
  };

  constexpr HalfVector2(){};

  constexpr HalfVector2(const Vector2& a)
      : x(ScalarToHalf(a.x)), y(ScalarToHalf(a.y)){};
};

static_assert(sizeof(Half) == sizeof(uint16_t));
static_assert(sizeof(HalfVector2) == 2 * sizeof(Half));
static_assert(sizeof(HalfVector3) == 3 * sizeof(Half));
static_assert(sizeof(HalfVector4) == 4 * sizeof(Half));

}  // namespace impeller
