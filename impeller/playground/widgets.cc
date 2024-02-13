// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "widgets.h"

namespace impeller {
std::tuple<Point, Point> DrawPlaygroundLine(Point default_position_a,
                                            Point default_position_b,
                                            Scalar radius,
                                            Color color_a,
                                            Color color_b) {
  Point position_a = default_position_a;
  Point position_b = default_position_b;
  float r_ = radius;
  Color color_a_ = color_a;
  Color color_b_ = color_b;

  position_a = IMPELLER_PLAYGROUND_POINT(position_a, r_, color_a_);
  position_b = IMPELLER_PLAYGROUND_POINT(position_b, r_, color_b_);

  auto dir = (position_b - position_a).Normalize() * r_;
  auto line_a = position_a + dir;
  auto line_b = position_b - dir;
  ImGui::GetBackgroundDrawList()->AddLine(
      {line_a.x, line_a.y}, {line_b.x, line_b.y},
      ImColor(color_b.red, color_b.green, color_b.blue, 0.3f));

  return std::make_tuple(position_a, position_b);
}
//

}  // namespace impeller
