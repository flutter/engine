// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/paint_region.h"

#include "flutter/display_list/utils/dl_bounds_accumulator.h"

namespace flutter {

DlFRect PaintRegion::ComputeBounds() const {
  RectBoundsAccumulator accumulator;
  for (const auto& r : *this) {
    accumulator.accumulate(r);
  }
  return accumulator.bounds();
}

}  // namespace flutter
