// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_GEOMETRY_SUPERELLIPSE_GEOMETRY_H_
#define FLUTTER_IMPELLER_ENTITY_GEOMETRY_SUPERELLIPSE_GEOMETRY_H_

#include "impeller/entity/geometry/geometry.h"

namespace impeller {

// Geometry class that can generate vertices for a super ellipse.
class SuperellipseGeometry final : public Geometry {
 public:
  explicit SuperellipseGeometry(const Point& center,
                                Scalar radius,
                                int degree,
                                Scalar alpha,
                                Scalar beta);

  ~SuperellipseGeometry() = default;

  // |Geometry|
  bool CoversArea(const Matrix& transform, const Rect& rect) const override;

  // |Geometry|
  bool IsAxisAlignedRect() const override;

 private:
  // |Geometry|
  GeometryResult GetPositionBuffer(const ContentContext& renderer,
                                   const Entity& entity,
                                   RenderPass& pass) const override;

  // |Geometry|
  std::optional<Rect> GetCoverage(const Matrix& transform) const override;

  Point center_;
  // 4 is a rectellipse
  int degree_;
  Scalar radius_;
  Scalar alpha_;
  Scalar beta_;

  SuperellipseGeometry(const SuperellipseGeometry&) = delete;

  SuperellipseGeometry& operator=(const SuperellipseGeometry&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_GEOMETRY_SUPERELLIPSE_GEOMETRY_H_
