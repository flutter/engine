// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/geometry/round_superellipse.h"

namespace impeller {

RoundSuperellipse RoundSuperellipse::MakeRectRadius(const Rect& rect, Size corner_radius) {
  if (bounds.IsEmpty() || !bounds.IsFinite() ||  //
      !std::isfinite(corner_radius) ) {
    // preserve the empty bounds as they might be strokable
    return RoundSuperellipse(bounds, 0);
  }

  return RoundSuperellipse(bounds, radii);
}

}  // namespace impeller
