// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/entity/geometry/circle_tessellator.h"

#include "flutter/fml/logging.h"

namespace impeller {

std::vector<Trig> CircleTessellator::trigs_[kMaxDivisions + 1];

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
  if (pixel_radius > (kMaxDivisions - 1)) {
    return kMaxDivisions;
  }
  return static_cast<size_t>(ceil(pixel_radius));
}

const std::vector<Trig>& CircleTessellator::GetTrigForDivisions(
    size_t divisions) {
  FML_DCHECK(divisions > 0 && divisions <= kMaxDivisions);
  std::vector<Trig>& trigs = trigs_[divisions];
  trigs.reserve(divisions + 1);

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

void CircleTessellator::GenerateCircleTriangleStrip(
    const TessellatedPointProc& proc,
    const Point& center,
    Scalar radius) const {
  auto trigs = GetTrigForDivisions(quadrant_divisions_);

  for (auto& trig : trigs) {
    auto offset = trig * radius;
    proc({center.x - offset.x, center.y + offset.y});
    proc({center.x - offset.x, center.y - offset.y});
  }
  // The second half of the circle should be iterated in reverse, but
  // we can instead iterate forward and swap the x/y values of the
  // offset as the angles should be symmetric and thus should generate
  // symmetrically reversed offset vectors.
  for (auto& trig : trigs) {
    auto offset = trig * radius;
    proc({center.x + offset.y, center.y + offset.x});
    proc({center.x + offset.y, center.y - offset.x});
  }
}

void CircleTessellator::GenerateRoundCapLineTriangleStrip(
    const TessellatedPointProc& proc,
    const Point& p0,
    const Point& p1,
    Scalar radius) const {
  auto trigs = GetTrigForDivisions(quadrant_divisions_);
  auto along = p1 - p0;
  auto length = along.GetLength();
  if (length < kEhCloseEnough) {
    return GenerateCircleTriangleStrip(proc, p0, radius);
  }
  along *= radius / length;
  auto across = Point(-along.y, along.x);

  for (auto& trig : trigs) {
    auto relative_across = across * trig.cos;
    auto relative_along = along * trig.sin;
    proc({p0 + relative_across - relative_along});
    proc({p1 + relative_across + relative_along});
  }
  // The second half of the round caps should be iterated in reverse, but
  // we can instead iterate forward and swap the sin/cos values as they
  // should be symmetric.
  for (auto& trig : trigs) {
    auto relative_across = across * trig.sin;
    auto relative_along = along * trig.cos;
    proc({p0 - relative_across - relative_along});
    proc({p1 - relative_across + relative_along});
  }
}

}  // namespace impeller
