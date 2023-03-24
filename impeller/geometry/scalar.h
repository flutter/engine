// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cfloat>
#include <cstdint>
#include <type_traits>
#include <valarray>

#include "impeller/geometry/constants.h"

namespace impeller {

using Scalar = float;
using Half = uint16_t;

template <class T, class = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr T Absolute(const T& val) {
  return val >= T{} ? val : -val;
}

constexpr inline bool ScalarNearlyZero(Scalar x,
                                       Scalar tolerance = kEhCloseEnough) {
  return Absolute(x) <= tolerance;
}

constexpr inline bool ScalarNearlyEqual(Scalar x,
                                        Scalar y,
                                        Scalar tolerance = kEhCloseEnough) {
  return ScalarNearlyZero(x - y, tolerance);
}

struct Degrees;

struct Radians {
  Scalar radians = 0.0;

  constexpr Radians() = default;

  explicit constexpr Radians(Scalar p_radians) : radians(p_radians) {}
};

struct Degrees {
  Scalar degrees = 0.0;

  constexpr Degrees() = default;

  explicit constexpr Degrees(Scalar p_degrees) : degrees(p_degrees) {}

  constexpr operator Radians() const {
    return Radians{degrees * kPi / 180.0f};
  };
};

/// @brief Convert a scalar to a half precision float.
///
/// Can express numbers in the range of 2^-14 to 65504.
/// See also: https://gcc.gnu.org/onlinedocs/gcc-4.9.4/gcc/Half-Precision.html#:~:text=The%20__fp16%20type%20is,parameters%20of%20type%20__fp16%20.
Half ScalarToHalf(Scalar f);

}  // namespace impeller
