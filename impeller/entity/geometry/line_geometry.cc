// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/geometry/line_geometry.h"

#include "flutter/impeller/entity/geometry/circle_tessellator.h"

namespace impeller {

LineGeometry::LineGeometry(Point p0, Point p1, Scalar width, Cap cap)
    : p0_(p0), p1_(p1), width_(width), cap_(cap) {
  FML_DCHECK(width >= 0);
}

Scalar LineGeometry::ComputeHalfWidth(const Matrix& transform) const {
  auto determinant = transform.GetDeterminant();
  if (determinant == 0) {
    return 0.0f;
  }

  Scalar min_size = 1.0f / sqrt(std::abs(determinant));
  return std::max(width_, min_size) * 0.5f;
}

Point LineGeometry::ComputeAlongVector(const Matrix& transform,
                                       bool allow_zero_length) const {
  Scalar stroke_half_width = ComputeHalfWidth(transform);
  if (stroke_half_width < kEhCloseEnough) {
    return {};
  }

  Point along = p1_ - p0_;
  Scalar length = along.GetLength();
  if (length < kEhCloseEnough) {
    if (!allow_zero_length) {
      // We won't enclose any pixels unless the endpoints are extended
      return {};
    }
    return {stroke_half_width, 0};
  } else {
    return along * stroke_half_width / length;
  }
}

bool LineGeometry::ComputeCorners(Point corners[4],
                                  const Matrix& transform,
                                  bool extend_endpoints) const {
  Point along = ComputeAlongVector(transform, extend_endpoints);
  if (along.IsZero()) {
    return false;
  }

  Point across = {along.y, -along.x};
  corners[0] = p0_ - across;
  corners[1] = p1_ - across;
  corners[2] = p0_ + across;
  corners[3] = p1_ + across;
  if (extend_endpoints) {
    corners[0] -= along;
    corners[1] += along;
    corners[2] -= along;
    corners[3] += along;
  }
  return true;
}

PrimitiveType LineGeometry::FillRectCapVertices(std::vector<Point>& vertices,
                                                const Matrix& transform) const {
  Point corners[4];
  if (ComputeCorners(corners, transform, cap_ == Cap::kSquare)) {
    vertices.reserve(4);
    vertices.push_back(corners[0]);
    vertices.push_back(corners[1]);
    vertices.push_back(corners[2]);
    vertices.push_back(corners[3]);
  }
  return PrimitiveType::kTriangleStrip;
}

PrimitiveType LineGeometry::FillRoundCapVertices(
    std::vector<Point>& vertices,
    const Matrix& transform) const {
  Point along = ComputeAlongVector(transform, true);
  if (!along.IsZero()) {
    Point across = {along.y, -along.x};

    CircleTessellator divider(transform, along.GetLength());

    size_t line_vertex_count = (p0_ != p1_) ? 6 : 0;
    auto point_count = divider.GetQuadrantVertexCount() * 4 + line_vertex_count;
    vertices.reserve(point_count);
    divider.FillQuadrantTriangles(vertices, p0_, -across, -along);
    divider.FillQuadrantTriangles(vertices, p0_, -along, across);
    if (p0_ != p1_) {
      // This would require fewer vertices with a triangle strip, but the
      // round caps cannot use that mode so we have to list 2 full triangles.
      vertices.emplace_back(p0_ + across);
      vertices.emplace_back(p1_ + across);
      vertices.emplace_back(p0_ - across);
      vertices.emplace_back(p1_ + across);
      vertices.emplace_back(p0_ - across);
      vertices.emplace_back(p1_ - across);
    }
    divider.FillQuadrantTriangles(vertices, p1_, across, along);
    divider.FillQuadrantTriangles(vertices, p1_, along, -across);
    FML_DCHECK(vertices.size() == point_count);
  }

  return PrimitiveType::kTriangle;
}

GeometryResult LineGeometry::GetPositionBuffer(const ContentContext& renderer,
                                               const Entity& entity,
                                               RenderPass& pass) const {
  std::vector<Point> vertices;
  PrimitiveType triangle_type;

  if (cap_ == Cap::kRound) {
    triangle_type = FillRoundCapVertices(vertices, entity.GetTransform());
  } else {
    triangle_type = FillRectCapVertices(vertices, entity.GetTransform());
  }
  if (vertices.empty()) {
    return {};
  }

  static_assert(sizeof(Point) == 2 * sizeof(float));
  static_assert(alignof(Point) == alignof(float));
  return GeometryResult{
      .type = triangle_type,
      .vertex_buffer =
          {
              .vertex_buffer = pass.GetTransientsBuffer().Emplace(
                  vertices.data(), vertices.size() * sizeof(Point),
                  alignof(Point)),
              .vertex_count = vertices.size(),
              .index_type = IndexType::kNone,
          },
      .transform = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransform(),
      .prevent_overdraw = false,
  };
}

// |Geometry|
GeometryResult LineGeometry::GetPositionUVBuffer(Rect texture_coverage,
                                                 Matrix effect_transform,
                                                 const ContentContext& renderer,
                                                 const Entity& entity,
                                                 RenderPass& pass) const {
  std::vector<Point> vertices;
  PrimitiveType triangle_type;

  if (cap_ == Cap::kRound) {
    triangle_type = FillRoundCapVertices(vertices, entity.GetTransform());
  } else {
    triangle_type = FillRectCapVertices(vertices, entity.GetTransform());
  }
  if (vertices.empty()) {
    return {};
  }

  auto uv_transform =
      texture_coverage.GetNormalizingTransform() * effect_transform;

  std::vector<Point> data(vertices.size() * 2);
  for (auto i = 0u, j = 0u; i < data.size(); i += 2, j++) {
    data[i] = vertices[j];
    data[i + 1] = uv_transform * vertices[j];
  }

  static_assert(sizeof(Point) == 2 * sizeof(float));
  static_assert(alignof(Point) == alignof(float));
  return GeometryResult{
      .type = triangle_type,
      .vertex_buffer =
          {
              .vertex_buffer = pass.GetTransientsBuffer().Emplace(
                  data.data(), data.size() * sizeof(Point), alignof(Point)),
              .vertex_count = vertices.size(),
              .index_type = IndexType::kNone,
          },
      .transform = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransform(),
      .prevent_overdraw = false,
  };
}

GeometryVertexType LineGeometry::GetVertexType() const {
  return GeometryVertexType::kPosition;
}

std::optional<Rect> LineGeometry::GetCoverage(const Matrix& transform) const {
  Point corners[4];
  if (!ComputeCorners(corners, transform, cap_ != Cap::kButt)) {
    return {};
  }

  for (int i = 0; i < 4; i++) {
    corners[i] = transform * corners[i];
  }
  return Rect::MakePointBounds(std::begin(corners), std::end(corners));
}

bool LineGeometry::CoversArea(const Matrix& transform, const Rect& rect) const {
  if (!transform.IsTranslationScaleOnly() || !IsAxisAlignedRect()) {
    return false;
  }
  auto coverage = GetCoverage(transform);
  return coverage.has_value() ? coverage->Contains(rect) : false;
}

bool LineGeometry::IsAxisAlignedRect() const {
  return cap_ != Cap::kRound && (p0_.x == p1_.x || p0_.y == p1_.y);
}

}  // namespace impeller
