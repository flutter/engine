// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/geometry/round_superellipse.h"

#include <cmath>

namespace impeller {

RoundSuperellipse RoundSuperellipse::MakeRectRadius(const Rect& rect,
                                                    Scalar corner_radius) {
  if (rect.IsEmpty() || !rect.IsFinite() ||  //
      !std::isfinite(corner_radius)) {
    // preserve the empty bounds as they might be strokable
    return RoundSuperellipse(rect, 0);
  }

  return RoundSuperellipse(rect, corner_radius);
}

}  // namespace impeller
