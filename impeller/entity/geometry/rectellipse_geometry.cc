// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "flutter/impeller/entity/geometry/rectellipse_geometry.h"

#include "impeller/geometry/constants.h"

namespace impeller {

RectellipseGeometry::RectellipseGeometry(const Point& center,
                                         Scalar radius,
                                         Scalar alpha,
                                         Scalar beta,
                                         Scalar n)
    : center_(center), radius_(radius), alpha_(alpha), beta_(beta), n_(n) {}

GeometryResult RectellipseGeometry::GetPositionBuffer(
    const ContentContext& renderer,
    const Entity& entity,
    RenderPass& pass) const {
  // https://math.stackexchange.com/questions/2573746/superellipse-parametric-equation
  Scalar a = alpha_;
  Scalar b = beta_;
  Scalar n = 4;

  // TODO(jonahwilliams): determine parameter values based on scaling factor.
  Scalar step = kPi / 80;

  // Generate the points for the top left quadrant, and then mirror to the other
  // quadrants.
  std::vector<Point> points;
  for (Scalar t = 0; t < (kPi / 2) - step; t += step) {
    Scalar x = a * pow(abs(cos(t)), 2 / n);
    Scalar y = b * pow(abs(sin(t)), 2 / n);
    points.emplace_back(x, y);
  }
  Scalar x = a * pow(abs(cos(kPi / 2)), 2 / n);
  Scalar y = b * pow(abs(sin(kPi / 2)), 2 / n);
  points.emplace_back(x, y);

  // Reflect into the other 3 quadrants and generate the tessellated mesh.
  std::vector<Point> geometry;
  static constexpr Point reflection[4] = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};

  geometry.push_back(center_);
  for (auto sc : reflection) {
    for (auto pt : points) {
      geometry.push_back(center_ + ((sc * pt) * radius_));
    }
  }

  std::vector<uint16_t> indices;
  for (auto i = 2u; i < geometry.size(); i++) {
    indices.push_back(0);
    indices.push_back(i - 1);
    indices.push_back(i);
  }

  auto& host_buffer = renderer.GetTransientsBuffer();
  return GeometryResult{
      .type = PrimitiveType::kTriangle,
      .vertex_buffer =
          {
              .vertex_buffer = host_buffer.Emplace(
                  geometry.data(), geometry.size() * sizeof(Point),
                  alignof(Point)),
              .index_buffer = host_buffer.Emplace(
                  indices.data(), indices.size() * sizeof(uint16_t),
                  alignof(uint16_t)),
              .vertex_count = indices.size(),
              .index_type = IndexType::k16bit,
          },
      .transform = entity.GetShaderTransform(pass),
  };
}

std::optional<Rect> RectellipseGeometry::GetCoverage(
    const Matrix& transform) const {
  return Rect::MakeMaximum();
}

bool RectellipseGeometry::CoversArea(const Matrix& transform,
                                     const Rect& rect) const {
  return false;
}

bool RectellipseGeometry::IsAxisAlignedRect() const {
  return false;
}

}  // namespace impeller
