// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_GEOMETRY_VERTEX_WRITER_H_
#define FLUTTER_IMPELLER_GEOMETRY_VERTEX_WRITER_H_

#include <vector>

#include "impeller/geometry/point.h"

namespace impeller {

class VertexWriter {
 public:
  explicit VertexWriter(std::vector<Point>& points,
                        std::vector<uint16_t>& indices)
      : points_(points), indices_(indices) {}

  ~VertexWriter() = default;

  void EndContour() {
    if (points_.size() == 0u || contour_start_ == points_.size() - 1) {
      // Empty or first contour.
      return;
    }

    auto start = contour_start_;
    auto end = points_.size() - 1;
    // Some polygons will not self close and an additional triangle
    // must be inserted, others will self close and we need to avoid
    // inserting an extra triangle.
    if (points_[end] == points_[start]) {
      end--;
    }

    if (contour_start_ > 0) {
      // Triangle strip break.
      indices_.emplace_back(indices_.back());
      indices_.emplace_back(start);
      indices_.emplace_back(start);

      // If the contour has an odd number of points, insert an extra point when
      // bridging to the next contour to preserve the correct triangle winding
      // order.
      if (previous_contour_odd_points_) {
        indices_.emplace_back(start);
      }
    } else {
      indices_.emplace_back(start);
    }

    size_t a = start + 1;
    size_t b = end;
    while (a < b) {
      indices_.emplace_back(a);
      indices_.emplace_back(b);
      a++;
      b--;
    }
    if (a == b) {
      indices_.emplace_back(a);
      previous_contour_odd_points_ = false;
    } else {
      previous_contour_odd_points_ = true;
    }
    contour_start_ = points_.size();
  }

  void Write(Point point) { points_.emplace_back(point); }

 private:
  bool previous_contour_odd_points_ = false;
  size_t contour_start_ = 0u;
  std::vector<Point>& points_;
  std::vector<uint16_t>& indices_;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_GEOMETRY_VERTEX_WRITER_H_
