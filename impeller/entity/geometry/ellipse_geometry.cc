// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/geometry/ellipse_geometry.h"

#include "flutter/impeller/entity/geometry/circle_tessellator.h"

namespace impeller {

EllipseGeometry::EllipseGeometry(Point center, Scalar radius)
    : center_(center), radius_(radius) {
  FML_DCHECK(radius >= 0);
}

GeometryResult EllipseGeometry::GetPositionBuffer(
    const ContentContext& renderer,
    const Entity& entity,
    RenderPass& pass) const {
  auto& host_buffer = pass.GetTransientsBuffer();

  CircleTessellator divider(entity.GetTransform(), radius_);
  auto points = divider.GetCircleTriangles(center_, radius_);

  return GeometryResult{
      .type = PrimitiveType::kTriangle,
      .vertex_buffer =
          {
              .vertex_buffer = host_buffer.Emplace(
                  points.data(), points.size() * sizeof(Point), alignof(float)),
              .vertex_count = points.size(),
              .index_type = IndexType::kNone,
          },
      .transform = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransform(),
      .prevent_overdraw = false,
  };
}

// |Geometry|
GeometryResult EllipseGeometry::GetPositionUVBuffer(
    Rect texture_coverage,
    Matrix effect_transform,
    const ContentContext& renderer,
    const Entity& entity,
    RenderPass& pass) const {
  auto& host_buffer = pass.GetTransientsBuffer();

  CircleTessellator divider(entity.GetTransform(), radius_);
  auto points = divider.GetCircleTriangles(center_, radius_);
  auto uv_transform =
      texture_coverage.GetNormalizingTransform() * effect_transform;

  std::vector<Point> data(points.size() * 2);
  for (auto i = 0u, j = 0u; j < points.size(); i += 2, j++) {
    data[i] = points[j];
    data[i + 1] = uv_transform * points[j];
  }

  return GeometryResult{
      .type = PrimitiveType::kTriangle,
      .vertex_buffer =
          {
              .vertex_buffer = host_buffer.Emplace(
                  data.data(), data.size() * sizeof(Point), alignof(float)),
              .vertex_count = points.size(),
              .index_type = IndexType::kNone,
          },
      .transform = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransform(),
      .prevent_overdraw = false,
  };
}

GeometryVertexType EllipseGeometry::GetVertexType() const {
  return GeometryVertexType::kPosition;
}

std::optional<Rect> EllipseGeometry::GetCoverage(
    const Matrix& transform) const {
  Point corners[4]{
      {center_.x, center_.y - radius_},
      {center_.x + radius_, center_.y},
      {center_.x, center_.y + radius_},
      {center_.x - radius_, center_.y},
  };

  for (int i = 0; i < 4; i++) {
    corners[i] = transform * corners[i];
  }
  return Rect::MakePointBounds(std::begin(corners), std::end(corners));
}

bool EllipseGeometry::CoversArea(const Matrix& transform,
                                 const Rect& rect) const {
  return false;
}

bool EllipseGeometry::IsAxisAlignedRect() const {
  return false;
}

}  // namespace impeller
