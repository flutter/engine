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

Vector2 LineGeometry::ComputeAlongVector(const Matrix& transform,
                                         bool allow_zero_length) const {
  Scalar stroke_half_width = ComputeHalfWidth(transform);
  if (stroke_half_width < kEhCloseEnough) {
    return {};
  }

  auto along = p1_ - p0_;
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
  auto along = ComputeAlongVector(transform, extend_endpoints);
  if (along.IsZero()) {
    return false;
  }

  auto across = Vector2(along.y, -along.x);
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

GeometryResult LineGeometry::GetPositionBuffer(const ContentContext& renderer,
                                               const Entity& entity,
                                               RenderPass& pass) const {
  auto& transform = entity.GetTransform();
  auto radius = ComputeHalfWidth(transform);

  VertexBufferBuilder<SolidFillVertexShader::PerVertexData> vtx_builder;

  if (cap_ == Cap::kRound) {
    CircleTessellator tessellator(transform, radius);
    vtx_builder.Reserve(tessellator.GetCircleVertexCount());
    tessellator.GenerateRoundCapLineTriangleStrip(
        [&vtx_builder](const Point& p) {  //
          vtx_builder.AppendVertex({.position = p});
        },
        p0_, p1_, radius);
  } else {
    Point corners[4];
    if (ComputeCorners(corners, transform, cap_ == Cap::kSquare)) {
      vtx_builder.Reserve(4);
      vtx_builder.AppendVertex({.position = corners[0]});
      vtx_builder.AppendVertex({.position = corners[1]});
      vtx_builder.AppendVertex({.position = corners[2]});
      vtx_builder.AppendVertex({.position = corners[3]});
    }
  }
  if (!vtx_builder.HasVertices()) {
    return {};
  }

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer =
          vtx_builder.CreateVertexBuffer(pass.GetTransientsBuffer()),
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
  auto& transform = entity.GetTransform();
  auto radius = ComputeHalfWidth(transform);

  auto uv_transform =
      texture_coverage.GetNormalizingTransform() * effect_transform;
  VertexBufferBuilder<TextureFillVertexShader::PerVertexData> vtx_builder;

  if (cap_ == Cap::kRound) {
    CircleTessellator tessellator(transform, radius);
    vtx_builder.Reserve(tessellator.GetCircleVertexCount());
    tessellator.GenerateRoundCapLineTriangleStrip(
        [&vtx_builder, &uv_transform](const Point& p) {
          vtx_builder.AppendVertex({
              .position = p,
              .texture_coords = uv_transform * p,
          });
        },
        p0_, p1_, radius);
  } else {
    Point corners[4];
    if (ComputeCorners(corners, transform, cap_ == Cap::kSquare)) {
      vtx_builder.Reserve(4);
      for (auto& corner : corners) {
        vtx_builder.AppendVertex({
            .position = corner,
            .texture_coords = uv_transform * corner,
        });
      }
    }
  }
  if (!vtx_builder.HasVertices()) {
    return {};
  }

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer =
          vtx_builder.CreateVertexBuffer(pass.GetTransientsBuffer()),
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
