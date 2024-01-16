// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_GEOMETRY_SATURATED_MATH_H_
#define FLUTTER_IMPELLER_GEOMETRY_SATURATED_MATH_H_

#include <limits>
#include <type_traits>

#include "impeller/geometry/scalar.h"

namespace impeller {

namespace saturated {

#define ONLY_ON_SIGNED_INT_RET(Type, Ret) \
  template <typename Type>                \
  constexpr inline std::enable_if_t<      \
      std::is_integral_v<Type> && std::is_signed_v<Type>, Ret>
#define ONLY_ON_SIGNED_INT(Type) ONLY_ON_SIGNED_INT_RET(Type, Type)

#define ONLY_ON_FLOAT_RET(Type, Ret) \
  template <typename Type>           \
  constexpr inline std::enable_if_t<std::is_floating_point_v<Type>, Ret>
#define ONLY_ON_FLOAT(Type) ONLY_ON_FLOAT_RET(Type, Type)

ONLY_ON_SIGNED_INT(SI) Add(SI location, SI distance) {
  SI result = location + distance;
  if (distance <= static_cast<SI>(0)) {
    // result should be <= location
    if (result > location) {
      // We must have had 2's complement underflow, return U_MIN
      return std::numeric_limits<SI>::min();
    }
  } else {
    // result should be >= location
    if (result < location) {
      // We must have had 2's complement overflow, return U_MAX
      return std::numeric_limits<SI>::max();
    }
  }
  return result;
}

ONLY_ON_FLOAT(FP) Add(FP location, FP distance) {
  return location + distance;
}

ONLY_ON_SIGNED_INT(SI) Sub(SI upper, SI lower) {
  SI result = upper - lower;
  if (upper >= lower) {
    // result should be >= 0
    if (result < static_cast<SI>(0)) {
      // We must have had 2's complement overflow, return T_MAX
      return std::numeric_limits<SI>::max();
    }
  } else {
    // result should be <= 0
    if (result > static_cast<SI>(0)) {
      // We must have had 2's complement underflow, return T_MIN
      return std::numeric_limits<SI>::min();
    }
  }
  return result;
}

ONLY_ON_FLOAT(FP) Sub(FP upper, FP lower) {
  return upper - lower;
}

ONLY_ON_SIGNED_INT_RET(SI, Scalar) AverageScalar(SI a, SI b) {
  // scalbn has an implementation for ints that converts to double
  // while adjusting the exponent.
  return static_cast<Scalar>(std::scalbn(a, -1) + std::scalbn(b, -1));
}

ONLY_ON_FLOAT(FP) AverageScalar(FP a, FP b) {
  // GetCenter might want this to return 0 for a Maximum Rect, but it
  // will currently produce NaN instead.  For the Maximum Rect itself
  // a 0 would make sense as the center, but for a computed rect that
  // incidentally ended up with infinities, NaN may be a better choice.
  return std::scalbn(a + b, -1);
}

#undef ONLY_ON_FLOAT
#undef ONLY_ON_FLOAT_RET
#undef ONLY_ON_SIGNED_INT
#undef ONLY_ON_SIGNED_INT_RET

}  // namespace saturated

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_GEOMETRY_SATURATED_MATH_H_
