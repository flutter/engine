// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_round_rect.h"

#include "fml/logging.h"

namespace flutter {

void DlFRRect::SetRectRadii(const DlFRect& rect, const DlFVector radii[4]) {
  if (!rect.is_finite()) {
    SetEmpty();
    return;
  }
  rect_ = rect.MakeSorted();
  if (rect_.is_empty()) {
    SetEmpty();
    return;
  }
  bool all_zero = true;
  bool all_same = true;
  for (int i = 0; i < 4; i++) {
    DlFVector r = radii[i];
    DlScalar x = r.x();
    DlScalar y = r.y();
    if (DlScalars_AreFinite(x, y)) {
      if (x <= 0.0 || y <= 0.0 || x + y == x || y + x == y) {
        // One of the radii are non-positive, or so close to zero
        // that they can't be distinguished vs the other, so we
        // set both to zero to prevent confusion.
        radii_[i] = {};
      } else {
        radii_[i] = {x, y};
        all_zero = false;
      }
      if (all_same && i > 0 && radii_[i] != radii_[0]) {
        all_same = false;
      }
    } else {
      ClearRadii();
      all_zero = true;
      break;
    }
  }
  if (all_zero) {
    type_ = Type::kRect;
    return;
  }
  auto min_scale = [](double s, double t, double v) {
    return std::min(s, t / v);
  };
  double x_scale, y_scale;
  x_scale = min_scale(2.0, rect_.width(), radii_[0].x() + radii[1].x());
  y_scale = min_scale(2.0, rect_.height(), radii_[1].y() + radii[2].y());
  if (!all_same) {
    x_scale = min_scale(x_scale, rect_.width(), radii_[2].x() + radii[3].x());
    y_scale = min_scale(y_scale, rect_.height(), radii_[3].y() + radii[0].y());
  }
  if (x_scale == 1.0 && y_scale == 1.0) {
    type_ = Type::kOval;
    return;
  }
  DlScalar scale = std::min(x_scale, y_scale);
  if (scale < 1.0) {
    for (int i = 0; i < 4; i++) {
      radii_[i] = radii_[i] * scale;
    }
  }
  if (all_same && radii_[0].x() == radii_[0].y()) {
    type_ = Type::kSimple;
  } else {
    if (radii_[0].y() == radii_[1].y() &&  //
        radii_[1].x() == radii_[2].x() &&  //
        radii_[2].y() == radii_[3].y() &&  //
        radii_[3].x() == radii_[0].x()) {
      type_ = Type::kNinePatch;
    } else {
      type_ = Type::kComplex;
    }
  }
  type_ = all_same ? Type::kSimple : Type::kComplex;
}

}  // namespace flutter
