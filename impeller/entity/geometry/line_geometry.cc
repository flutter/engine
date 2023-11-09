// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/geometry/line_geometry.h"

namespace impeller {

LineGeometry::LineGeometry(Point p0, Point p1, Scalar width, Cap cap)
    : p0_(p0), p1_(p1) {
  Point along = (p1 - p0).Normalize();
  if (width < 1) {
    width = 1;
  }
  along *= width * 0.5f;
  Point across = {along.y, -along.x};
  switch (cap) {
    case Cap::kButt:
      corners_[0] = p0 - across;
      corners_[1] = p1 - across;
      corners_[2] = p0 + across;
      corners_[3] = p1 + across;
      break;

    case Cap::kSquare:
    case Cap::kRound:
      corners_[0] = p0 - across - along;
      corners_[1] = p1 - across + along;
      corners_[2] = p0 + across - along;
      corners_[3] = p1 + across + along;
  }
}

LineGeometry::~LineGeometry() = default;

GeometryResult LineGeometry::GetPositionBuffer(const ContentContext& renderer,
                                               const Entity& entity,
                                               RenderPass& pass) {
  auto& host_buffer = pass.GetTransientsBuffer();
  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer =
          {
              .vertex_buffer = host_buffer.Emplace(corners_, 8 * sizeof(float),
                                                   alignof(float)),
              .vertex_count = 4,
              .index_type = IndexType::kNone,
          },
      .transform = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransformation(),
      .prevent_overdraw = false,
  };
}

// |Geometry|
GeometryResult LineGeometry::GetPositionUVBuffer(Rect texture_coverage,
                                                 Matrix effect_transform,
                                                 const ContentContext& renderer,
                                                 const Entity& entity,
                                                 RenderPass& pass) {
  auto& host_buffer = pass.GetTransientsBuffer();

  auto uv_transform =
      texture_coverage.GetNormalizingTransform() * effect_transform;
  std::vector<Point> data(8);
  for (auto i = 0u, j = 0u; i < 8; i += 2, j++) {
    data[i] = corners_[j];
    data[i + 1] = uv_transform * corners_[j];
  }

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer =
          {
              .vertex_buffer = host_buffer.Emplace(
                  data.data(), 16 * sizeof(float), alignof(float)),
              .vertex_count = 4,
              .index_type = IndexType::kNone,
          },
      .transform = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransformation(),
      .prevent_overdraw = false,
  };
}

GeometryVertexType LineGeometry::GetVertexType() const {
  return GeometryVertexType::kPosition;
}

std::optional<Rect> LineGeometry::GetCoverage(const Matrix& transform) const {
  auto rect = Rect::MakePointBounds(std::begin(corners_), std::end(corners_));
  return rect.has_value() ? rect->TransformBounds(transform) : rect;
}

bool LineGeometry::CoversArea(const Matrix& transform, const Rect& rect) const {
  if (!transform.IsTranslationScaleOnly() || !IsAxisAlignedRect()) {
    return false;
  }
  auto coverage = GetCoverage(transform);
  return coverage.has_value() ? coverage->Contains(rect) : false;
}

bool LineGeometry::IsAxisAlignedRect() const {
  return p0_.x == p1_.x || p0_.y == p1_.y;
}

}  // namespace impeller
