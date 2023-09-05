// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_round_rect.h"

#include "fml/logging.h"

namespace flutter {

DlFRRect DlFRRect::MakeRectRadii(const DlFRect& in_rect,
                                 const DlFVector in_radii[4]) {
  if (!in_rect.is_finite()) {
    return DlFRRect();
  }
  DlFRect sorted = in_rect.Sorted();
  if (sorted.is_empty()) {
    return DlFRRect();
  }
  bool all_zero = true;
  bool all_same = true;

  DlFVector radii[4];
  for (int i = 0; i < 4; i++) {
    DlFVector r = in_radii[i];
    DlScalar x = r.x();
    DlScalar y = r.y();
    if (DlScalars_AreFinite(x, y)) {
      if (x <= 0.0 || y <= 0.0 || x + y == x || y + x == y) {
        // One of the radii are non-positive, or so close to zero
        // that they can't be distinguished vs the other, so we
        // set both to zero to prevent confusion.
        radii[i] = DlFVector();
      } else {
        radii[i] = {x, y};
        all_zero = false;
      }
      if (all_same && i > 0 && radii[i] != radii[0]) {
        all_same = false;
      }
    } else {
      all_zero = true;
      break;
    }
  }
  if (all_zero) {
    return DlFRRect(sorted);
  }
  auto min_scale = [](double s, double t, double v) {
    return std::min(s, t / v);
  };
  double x_scale, y_scale;
  x_scale = min_scale(2.0, sorted.width(), radii[0].x() + radii[1].x());
  y_scale = min_scale(2.0, sorted.height(), radii[1].y() + radii[2].y());
  if (!all_same) {
    x_scale = min_scale(x_scale, sorted.width(), radii[2].x() + radii[3].x());
    y_scale = min_scale(y_scale, sorted.height(), radii[3].y() + radii[0].y());
  }
  if (x_scale == 1.0 && y_scale == 1.0) {
    return DlFRRect(sorted, Type::kOval);
  }
  DlScalar scale = std::min(x_scale, y_scale);
  if (scale < 1.0) {
    for (int i = 0; i < 4; i++) {
      radii[i] = radii[i] * scale;
    }
  }
  Type type;
  if (all_same) {
    if (radii[0].x() == radii[0].y()) {
      type = Type::kCircularCorners;
    } else {
      type = Type::kOvalCorners;
    }
  } else {
    if (radii[0].y() == radii[1].y() &&  //
        radii[1].x() == radii[2].x() &&  //
        radii[2].y() == radii[3].y() &&  //
        radii[3].x() == radii[0].x()) {
      type = Type::kNinePatch;
    } else {
      type = Type::kComplex;
    }
  }
  return DlFRRect(sorted, radii, type);
}

DlFRRect DlFRRect::MakeRectXY(const DlFRect& in_rect,
                              DlScalar dx,
                              DlScalar dy) {
  if (!in_rect.is_finite()) {
    return DlFRRect();
  }
  DlFRect sorted = in_rect.Sorted();
  if (sorted.is_empty()) {
    return DlFRRect();
  }
  if (!DlScalars_AreFinite(dx, dy) ||  //
      (dx <= 0 || dy <= 0) ||          //
      dx + dy == dx || dy + dx == dy) {
    // One of the radii are non-positive, or so close to zero
    // that they can't be distinguished vs the other, so we
    // set act as if both are zero to prevent confusion.
    return DlFRRect(sorted);
  }
  auto min_scale = [](double s, double t, double v) {
    return std::min(s, t / v);
  };
  double x_scale, y_scale;
  x_scale = min_scale(2.0, sorted.width(), dx * 2.0);
  y_scale = min_scale(2.0, sorted.height(), dy * 2.0);
  DlScalar scale = std::min(x_scale, y_scale);
  if (scale < 1.0) {
    dx *= scale;
    dy *= scale;
  }
  Type type;
  // dx and dy are now scaled proportionally to not overflow the
  // rect, but in case of bit-errors in the math we use ">="
  // comparisons to more robustly detect the oval case.
  if (dx + dx >= sorted.width() && dy + dy >= sorted.height()) {
    dx = sorted.width() * 0.5f;
    dy = sorted.height() * 0.5f;
    type = Type::kOval;
  } else if (dx == dy) {
    type = Type::kCircularCorners;
  } else {
    type = Type::kOvalCorners;
  }
  DlFVector radii[4] = {
      {dx, dy},
      {dx, dy},
      {dx, dy},
      {dx, dy},
  };
  return DlFRRect(sorted, radii, type);
}

// For coordinate (rel_x, rel_y) relative to a given corner, is the point
// inside the distorted oval with radii (rad_x, rad_y) whose center is at
// a relative position of (rad_x, rad_y) from the corner.
static bool CornerContains(DlScalar rel_x, DlScalar rel_y, DlFVector radii) {
  DlScalar rad_x = radii.x();
  DlScalar rad_y = radii.y();

  // Make the coordinate relative to the origin of the oval with positive
  // values pointing at the corner. We already know that rel_x/y are within
  // the bounds, so their relative position to the origin of the oval must
  // be <= rad_x/y
  rel_x = rad_x - rel_x;
  rel_y = rad_y - rel_y;
  if (rel_x <= 0 || rel_y <= 0) {
    return true;
  }
  // ((rel_x / rad_x) ^ 2 + (rel_y / rad_y) ^ 2) <= 1.0
  // (rel_x^2 * rad_y^2) + (rel_y^2 * rad_x^2) / (rad_x^2 * rad_y^2) <= 1.0
  // (rel_x^2 * rad_y^2) + (rel_y^2 * rad_x^2) <= (rad_x^2 * rad_y^2)
  rel_x = rel_x * rel_x;
  rel_y = rel_y * rel_y;
  rad_x = rad_x * rad_x;
  rad_y = rad_y * rad_y;
  return (rel_x * rad_y + rel_y * rad_x) <= (rad_x * rad_y);
}

bool DlFRRect::Contains(const DlFPoint& p) const {
  if (!rect_.Contains(p)) {
    return false;
  }
  if (type_ <= Type::kRect) {
    return true;
  }

  DlScalar px_rel_left = p.x() - rect_.left();
  DlScalar px_rel_right = rect_.right() - p.x();
  DlScalar py_rel_top = p.y() - rect_.top();
  DlScalar py_rel_bottom = rect_.bottom() - p.y();

  return CornerContains(px_rel_left, py_rel_top, radii_[0]) &&
         CornerContains(px_rel_right, py_rel_top, radii_[1]) &&
         CornerContains(px_rel_right, py_rel_bottom, radii_[2]) &&
         CornerContains(px_rel_left, py_rel_bottom, radii_[3]);
}

bool DlFRRect::Contains(const DlFRect& r) const {
  if (!rect_.Contains(r)) {
    return false;
  }
  if (type_ <= Type::kRect) {
    return true;
  }

  DlScalar px_rel_left = r.left() - rect_.left();
  DlScalar px_rel_right = rect_.right() - r.right();
  DlScalar py_rel_top = r.top() - rect_.top();
  DlScalar py_rel_bottom = rect_.bottom() - r.bottom();

  return CornerContains(px_rel_left, py_rel_top, radii_[0]) &&
         CornerContains(px_rel_right, py_rel_top, radii_[1]) &&
         CornerContains(px_rel_right, py_rel_bottom, radii_[2]) &&
         CornerContains(px_rel_left, py_rel_bottom, radii_[3]);
}

std::ostream& operator<<(std::ostream& os, const DlFRRect& rrect) {
  switch (rrect.type()) {
    case DlFRRect::Type::kEmpty:
      return os << "DLFRRect(empty)";
    case DlFRRect::Type::kRect:
      return os << "DlFRRect(rect: " << rrect.rect() << ")";
    case DlFRRect::Type::kOval:
      return os << "DlFRRect(oval: " << rrect.rect() << ")";
    case DlFRRect::Type::kCircularCorners:
      return os << "DlFRRect(circular: " << rrect.rect() << ", "
                << rrect.upper_left_radii().x() << ")";
    case DlFRRect::Type::kOvalCorners:
      return os << "DlFRRect(elliptical: " << rrect.rect() << ", "
                << rrect.upper_left_radii() << ")";
    case DlFRRect::Type::kNinePatch:
      return os << "DlFRRect(nine patch: " << rrect.rect() << ", {"
                << "UL: " << rrect.upper_left_radii() << ", "
                << "LR: " << rrect.lower_right_radii() << "}";
    case DlFRRect::Type::kComplex:
      return os << "DlFRRect(complex: " << rrect.rect() << ", {"
                << "UL: " << rrect.upper_left_radii() << ", "
                << "UR: " << rrect.upper_right_radii() << ", "
                << "LR: " << rrect.lower_right_radii() << ", "
                << "LL: " << rrect.lower_left_radii() << "})";
  }
}

}  // namespace flutter
