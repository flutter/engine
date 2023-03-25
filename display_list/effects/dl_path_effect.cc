// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_path_effect.h"

namespace flutter {

dl_shared<DlPathEffect> DlPathEffect::MakeDash(const SkScalar* intervals,
                                               int count,
                                               SkScalar phase) {
  return DlDashPathEffect::Make(intervals, count, phase);
}

dl_shared<DlPathEffect> DlDashPathEffect::Make(const SkScalar* intervals,
                                               int count,
                                               SkScalar phase) {
  size_t needed = sizeof(DlDashPathEffect) + sizeof(SkScalar) * count;
  void* storage = ::operator new(needed);

  return dl_place_shared<DlDashPathEffect>(storage, intervals, count, phase);
}

std::optional<SkRect> DlDashPathEffect::effect_bounds(SkRect& rect) const {
  // The dashed path will always be a subset of the original.
  return rect;
}

}  // namespace flutter
