// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_WEB_UI_SKWASM_HELPERS_H_
#define FLUTTER_LIB_WEB_UI_SKWASM_HELPERS_H_

#include "render_strategy.h"

namespace Skwasm {

inline SkMatrix createMatrix(const Scalar* f) {
  return SkMatrix::MakeAll(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7],
                           f[8]);
}

inline Skwasm::RRect createRRect(const Scalar* f) {
  const Skwasm::Rect* rect = reinterpret_cast<const Rect*>(f);
  const Vector* radiiValues = reinterpret_cast<const Vector*>(f + 4);

  Skwasm::RRect rr;
  rr.setRectRadii(*rect, radiiValues);
  return rr;
}

// This needs to be kept in sync with the "FilterQuality" enum in dart:ui
enum class FilterQuality {
  none,
  low,
  medium,
  high,
};

inline FilterMode filterModeForQuality(FilterQuality quality) {
  switch (quality) {
    case FilterQuality::none:
    case FilterQuality::low:
      return FilterMode::kNearest;
    case FilterQuality::medium:
    case FilterQuality::high:
      return FilterMode::kLinear;
  }
}

inline SamplingOptions samplingOptionsForQuality(FilterQuality quality) {
  switch (quality) {
    case FilterQuality::none:
      return SamplingOptions(FilterMode::kNearest, MipmapMode::kNone);
    case FilterQuality::low:
      return SamplingOptions(FilterMode::kLinear, MipmapMode::kNone);
    case FilterQuality::medium:
      return SamplingOptions(FilterMode::kLinear, MipmapMode::kLinear);
    case FilterQuality::high:
      // Cubic equation coefficients recommended by Mitchell & Netravali
      // in their paper on cubic interpolation.
      return SamplingOptions(SkCubicResampler::Mitchell());
  }
}
}  // namespace Skwasm

#endif  // FLUTTER_LIB_WEB_UI_SKWASM_HELPERS_H_
