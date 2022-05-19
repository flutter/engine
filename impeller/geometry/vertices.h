// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <optional>
#include <vector>

#include "impeller/geometry/color.h"
#include "impeller/geometry/point.h"
#include "impeller/geometry/rect.h"

namespace impeller {

class Vertices {
 public:
  Vertices(std::vector<Point> points,
           std::vector<uint16_t> indexes,
           std::vector<Color> colors,
           Rect bounds);

  ~Vertices();

  std::optional<Rect> GetBoundingBox() const;

  std::vector<Point> get_points() const { return points_; }

  std::vector<uint16_t> get_indexes() const { return indexes_; }

  std::vector<Color> get_colors() const { return colors_; }

  std::vector<Point> points_;
  std::vector<uint16_t> indexes_;
  std::vector<Color> colors_;
  Rect bounds_;
};

}  // namespace impeller
