// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_GEOMETRY_SATURATED_MATH_H_
#define FLUTTER_IMPELLER_GEOMETRY_SATURATED_MATH_H_

#include <algorithm>
#include <limits>
#include <type_traits>

#include "flutter/fml/logging.h"
#include "impeller/geometry/scalar.h"

namespace impeller {

namespace saturated {

// NOLINTBEGIN(readability-identifier-naming)
template <typename T>
inline constexpr bool is_signed_integral_v =
    std::is_integral_v<T> && std::is_signed_v<T>;
// NOLINTEND(readability-identifier-naming)

#define ONLY_ON_SIGNED_INT_RET(Type, Ret) \
  template <typename Type>                \
  constexpr inline std::enable_if_t<is_signed_integral_v<Type>, Ret>
#define ONLY_ON_SIGNED_INT(Type) ONLY_ON_SIGNED_INT_RET(Type, Type)

#define ONLY_ON_FLOAT_RET(Type, Ret) \
  template <typename Type>           \
  constexpr inline std::enable_if_t<std::is_floating_point_v<Type>, Ret>
#define ONLY_ON_FLOAT(Type) ONLY_ON_FLOAT_RET(Type, Type)

#define ONLY_ON_FLOAT_TO_SIGNED_INT_RET(FPType, SIType, Ret) \
  template <typename FPType, typename SIType>                \
  constexpr inline std::enable_if_t<                         \
      std::is_floating_point_v<FPType> && is_signed_integral_v<SIType>, Ret>
#define ONLY_ON_FLOAT_TO_SIGNED_INT(FPType, SIType) \
  ONLY_ON_FLOAT_TO_SIGNED_INT_RET(FPType, SIType, SIType)

#define ONLY_ON_DIFFERING_FLOAT_RET(FPType1, FPType2, Ret)                   \
  template <typename FPType1, typename FPType2>                              \
  constexpr inline std::enable_if_t<std::is_floating_point_v<FPType1> &&     \
                                        std::is_floating_point_v<FPType2> && \
                                        !std::is_same_v<FPType1, FPType2>,   \
                                    Ret>
#define ONLY_ON_DIFFERING_FLOAT(FPType1, FPType2) \
  ONLY_ON_DIFFERING_FLOAT_RET(FPType1, FPType2, FPType2)

#define ONLY_ON_SAME_TYPES_RET(Type1, Type2, Ret) \
  template <typename Type1, typename Type2>       \
  constexpr inline std::enable_if_t<std::is_same_v<Type1, Type2>, Ret>
#define ONLY_ON_SAME_TYPES(Type1, Type2) \
  ONLY_ON_SAME_TYPES_RET(Type1, Type2, Type2)

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

ONLY_ON_SAME_TYPES(T, U) Cast(T v) {
  return v;
}

ONLY_ON_FLOAT_TO_SIGNED_INT(FP, SI) Cast(FP v) {
  if (v <= std::numeric_limits<SI>::min()) {
    return std::numeric_limits<SI>::min();
  } else if (v >= std::numeric_limits<SI>::max()) {
    return std::numeric_limits<SI>::max();
  }
  return static_cast<SI>(v);
}

ONLY_ON_DIFFERING_FLOAT(FP1, FP2) Cast(FP1 v) {
  if (std::isfinite(v)) {
    // Avoid truncation to inf/-inf.
    return std::clamp(static_cast<FP2>(v),  //
                      std::numeric_limits<FP2>::lowest(),
                      std::numeric_limits<FP2>::max());
  } else {
    return static_cast<FP2>(v);
  }
}

#undef ONLY_ON_SAME_TYPES
#undef ONLY_ON_SAME_TYPES_RET
#undef ONLY_ON_DIFFERING_FLOAT
#undef ONLY_ON_DIFFERING_FLOAT_RET
#undef ONLY_ON_FLOAT_TO_SIGNED_INT
#undef ONLY_ON_FLOAT_TO_SIGNED_INT_RET
#undef ONLY_ON_FLOAT
#undef ONLY_ON_FLOAT_RET
#undef ONLY_ON_SIGNED_INT
#undef ONLY_ON_SIGNED_INT_RET

}  // namespace saturated

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_GEOMETRY_SATURATED_MATH_H_
