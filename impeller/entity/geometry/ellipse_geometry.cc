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

  CircleTessellator tessellator(entity.GetTransform(), radius_);
  auto vertices = std::vector<Point>(tessellator.GetCircleVertexCount());
  tessellator.GenerateCircleTriangleStrip(
      [&vertices](const Point& p) { vertices.push_back(p); }, center_, radius_);

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer =
          {
              .vertex_buffer = host_buffer.Emplace(
                  vertices.data(), vertices.size() * sizeof(Point),
                  alignof(float)),
              .vertex_count = vertices.size(),
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

  auto uv_transform =
      texture_coverage.GetNormalizingTransform() * effect_transform;

  CircleTessellator tessellator(entity.GetTransform(), radius_);
  auto vertices = std::vector<Point>(tessellator.GetCircleVertexCount());
  tessellator.GenerateCircleTriangleStrip(
      [&vertices, &uv_transform](const Point& p) {
        vertices.push_back(p);
        vertices.push_back(uv_transform * p);
      },
      center_, radius_);

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer =
          {
              .vertex_buffer = host_buffer.Emplace(
                  vertices.data(), vertices.size() * sizeof(Point),
                  alignof(float)),
              .vertex_count = vertices.size() / 2,
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
