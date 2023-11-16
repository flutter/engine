// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/entity/geometry/circle_tessellator.h"

#include "flutter/fml/logging.h"

namespace impeller {

std::vector<Trig> CircleTessellator::trigs_[MAX_DIVISIONS_ + 1];

size_t CircleTessellator::ComputeQuadrantDivisions(Scalar pixel_radius) {
  // Note: these values are approximated based on the values returned from
  // the decomposition of 4 cubics performed by Path::CreatePolyline.
  if (pixel_radius < 1.0) {
    return 1;
  }
  if (pixel_radius < 2.0) {
    return 2;
  }
  if (pixel_radius < 12.0) {
    return 6;
  }
  if (pixel_radius <= 36.0) {
    return 9;
  }
  pixel_radius /= 4;
  if (pixel_radius > (MAX_DIVISIONS_ - 1)) {
    return MAX_DIVISIONS_;
  }
  return static_cast<int>(ceil(pixel_radius));
}

const std::vector<Trig>& CircleTessellator::GetTrigForDivisions(
    size_t divisions) {
  FML_DCHECK(divisions > 0 && divisions <= MAX_DIVISIONS_);
  std::vector<Trig>& trigs = trigs_[divisions];

  if (trigs.empty()) {
    double angle_scale = kPiOver2 / divisions;

    trigs.emplace_back(1.0, 0.0);
    for (size_t i = 1; i < divisions; i++) {
      trigs.emplace_back(Radians(i * angle_scale));
    }
    trigs.emplace_back(0.0, 1.0);

    FML_DCHECK(trigs.size() == divisions + 1);
  }

  return trigs;
}

void CircleTessellator::ExtendRelativeQuadrantToAbsoluteCircle(
    std::vector<Point>& points,
    const Point& center) {
  auto quadrant_points = points.size();

  // The 1st quadrant points are reversed in order, reflected around
  // the Y axis, and translated to become absolute 2nd quadrant points.
  for (size_t i = 1; i <= quadrant_points; i++) {
    auto point = points[quadrant_points - i];
    points.emplace_back(center.x + point.x, center.y - point.y);
  }

  // The 1st quadrant points are reflected around the X & Y axes
  // and translated to become absolute 3rd quadrant points.
  for (size_t i = 0; i < quadrant_points; i++) {
    auto point = points[i];
    points.emplace_back(center.x - point.x, center.y - point.y);
  }

  // The 1st quadrant points are reversed in order, reflected around
  // the X axis and translated to become absolute 4th quadrant points.
  // The 1st quadrant points are also translated to the center point as
  // well since this is the last time we will use them.
  for (size_t i = 1; i <= quadrant_points; i++) {
    auto point = points[quadrant_points - i];
    points.emplace_back(center.x - point.x, center.y + point.y);

    // This is the last loop where we need the first quadrant to be
    // relative so we convert them to absolute as we go.
    points[quadrant_points - i] = center + point;
  }
}

void CircleTessellator::FillQuadrantTriangles(std::vector<Point>& points,
                                              const Point& center,
                                              const Point& start_vector,
                                              const Point& end_vector) const {
  // We only deal with circles for now
  FML_DCHECK(start_vector.GetLength() - end_vector.GetLength() <
             kEhCloseEnough);
  // And only for perpendicular vectors
  FML_DCHECK(start_vector.Dot(end_vector) < kEhCloseEnough);

  auto trigs = GetTrigForDivisions(quadrant_divisions_);

  auto prev = center + (trigs[0].cos * start_vector +  //
                        trigs[0].sin * end_vector);
  for (size_t i = 1; i < trigs.size(); i++) {
    points.emplace_back(center);
    points.emplace_back(prev);
    prev = center + (trigs[i].cos * start_vector +  //
                     trigs[i].sin * end_vector);
    points.emplace_back(prev);
  }
}

std::vector<Point> CircleTessellator::GetCircleTriangles(const Point& center,
                                                         Scalar radius) const {
  std::vector<Point> points = std::vector<Point>();
  const size_t quadrant_points = quadrant_divisions_ * 3;
  points.reserve(quadrant_points * 4);

  // Start with the quadrant top-center to right-center using coordinates
  // relative to the (0, 0). The coordinates will be made absolute relative
  // to the center during the extend method below.
  FillQuadrantTriangles(points, {}, {0, -radius}, {radius, 0});
  FML_DCHECK(points.size() == quadrant_points);

  ExtendRelativeQuadrantToAbsoluteCircle(points, center);

  FML_DCHECK(points.size() == quadrant_points * 4);

  return points;
}

}  // namespace impeller
