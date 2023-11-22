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

  VertexBufferBuilder<SolidFillVertexShader::PerVertexData> vtx_builder;

  CircleTessellator tessellator(entity.GetTransform(), radius_);
  vtx_builder.Reserve(tessellator.GetCircleVertexCount());
  tessellator.GenerateCircleTriangleStrip(
      [&vtx_builder](const Point& p) {  //
        vtx_builder.AppendVertex({.position = p});
      },
      center_, radius_);

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer = vtx_builder.CreateVertexBuffer(host_buffer),
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
  VertexBufferBuilder<TextureFillVertexShader::PerVertexData> vtx_builder;

  CircleTessellator tessellator(entity.GetTransform(), radius_);
  vtx_builder.Reserve(tessellator.GetCircleVertexCount());
  tessellator.GenerateCircleTriangleStrip(
      [&vtx_builder, &uv_transform](const Point& p) {
        vtx_builder.AppendVertex({
            .position = p,
            .texture_coords = uv_transform * p,
        });
      },
      center_, radius_);

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer = vtx_builder.CreateVertexBuffer(host_buffer),
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
