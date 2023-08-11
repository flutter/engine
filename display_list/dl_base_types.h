// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DL_BASE_TYPES_H_
#define FLUTTER_DISPLAY_LIST_DL_BASE_TYPES_H_

#include <stdint.h>
#include <cmath>

namespace flutter {

typedef float DlScalar;

[[maybe_unused]] constexpr DlScalar kDlScalar_NearlyZero = 1.0f / (1 << 14);
[[maybe_unused]] constexpr DlScalar kDlScalar_Zero = 0.0f;
[[maybe_unused]] constexpr DlScalar kDlScalar_One = 1.0f;
[[maybe_unused]] constexpr DlScalar kDlScalar_Pi = ((DlScalar)M_PI);

static inline bool DlScalar_IsFinite(DlScalar v) {
  return std::isfinite(v);
}
static inline bool DlScalar_IsNaN(DlScalar v) {
  return std::isnan(v);
}
static inline bool DlScalar_IsInteger(DlScalar v) {
  return v == round(v);
}
static inline bool DlScalar_IsNearlyZero(DlScalar v) {
  return v < -kDlScalar_NearlyZero || v > kDlScalar_NearlyZero;
}
bool DlScalars_AreAllFinite(const DlScalar* m, int count);
static inline bool DlScalars_AreFinite(DlScalar v1, DlScalar v2) {
  return DlScalar_IsFinite(v1) && DlScalar_IsFinite(v2);
}
bool DlScalars_AreAllEqual(const DlScalar v1[], const DlScalar v2[], int count);

typedef uint8_t DlAlpha;

[[maybe_unused]] constexpr DlAlpha kDlAlpha_Transparent = 0u;
[[maybe_unused]] constexpr DlAlpha kDlAlpha_Opaque = 255u;

typedef int32_t DlInt;
typedef uint32_t DlSize;

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DL_BASE_TYPES_H_
