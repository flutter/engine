// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/geometry/point_field_geometry.h"

#include "impeller/core/vertex_buffer.h"
#include "impeller/entity/geometry/geometry.h"
#include "impeller/renderer/command_buffer.h"

namespace impeller {

PointFieldGeometry::PointFieldGeometry(std::vector<Point> points,
                                       Scalar radius,
                                       bool round)
    : points_(std::move(points)), radius_(radius), round_(round) {}

GeometryResult PointFieldGeometry::GetPositionBuffer(
    const ContentContext& renderer,
    const Entity& entity,
    RenderPass& pass) const {
  if (radius_ < 0.0) {
    return {};
  }
  Matrix transform = entity.GetTransform();
  Scalar determinant = transform.GetDeterminant();
  if (determinant == 0) {
    return {};
  }

  Scalar min_size = 1.0f / sqrt(std::abs(determinant));
  Scalar radius = std::max(radius_, min_size);

  HostBuffer& host_buffer = renderer.GetTransientsBuffer();
  if (round_) {
    // Get triangulation relative to {0, 0} so we can translate it to each
    // point in turn.
    auto generator =
        renderer.GetTessellator()->FilledCircle(transform, {}, radius);
    FML_DCHECK(generator.GetTriangleType() == PrimitiveType::kTriangleStrip);
    std::vector<Point> circle_vertices;
    circle_vertices.reserve(generator.GetVertexCount());
    generator.GenerateVertices([&circle_vertices](const Point& p) {  //
      circle_vertices.push_back(p);
    });
    FML_DCHECK(circle_vertices.size() == generator.GetVertexCount());

    size_t vertex_count = ((circle_vertices.size() + 2) * points_.size() - 2);
    BufferView vertex_data =
        host_buffer.Emplace(vertex_count, alignof(Point), [&](uint8_t* data) {
          Point* output = reinterpret_cast<Point*>(data);
          size_t offset = 0;

          for (auto& vertex : circle_vertices) {
            output[offset++] = vertex + points_[0];
          }

          for (auto i = 1u; i < vertex_count; i++) {
            output[offset++] = output[offset];
            output[offset++] = circle_vertices[0] + points_[i];
            for (auto& vertex : circle_vertices) {
              output[offset++] = vertex + points_[i];
            }
          }
        });

    return GeometryResult{
        .type = PrimitiveType::kTriangleStrip,
        .vertex_buffer = VertexBuffer{.vertex_buffer = std::move(vertex_data),
                                      .index_buffer = {},
                                      .vertex_count = vertex_count,
                                      .index_type = IndexType::kNone},
        .transform = entity.GetShaderTransform(pass),
    };
  }

  size_t vertex_count = 6 * points_.size() - 2;
  BufferView vertex_data =
      host_buffer.Emplace(vertex_count, alignof(Point), [&](uint8_t* data) {
        Point* output = reinterpret_cast<Point*>(data);
        size_t offset = 0;
        bool has_vertices = false;

        for (auto& point : points_) {
          Point first = Point(point.x - radius, point.y - radius);

          if (has_vertices) {
            output[offset++] = output[offset];
            output[offset++] = first;
          }

          // Z pattern from UL -> UR -> LL -> LR
          output[offset++] = first;
          output[offset++] = Point{point.x + radius, point.y - radius};
          output[offset++] = Point{point.x - radius, point.y + radius};
          output[offset++] = Point{point.x + radius, point.y + radius};
          has_vertices = true;
        }
      });

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer = VertexBuffer{.vertex_buffer = std::move(vertex_data),
                                    .index_buffer = {},
                                    .vertex_count = vertex_count,
                                    .index_type = IndexType::kNone},
      .transform = entity.GetShaderTransform(pass),
  };
}

// |Geometry|
GeometryVertexType PointFieldGeometry::GetVertexType() const {
  return GeometryVertexType::kPosition;
}

// |Geometry|
std::optional<Rect> PointFieldGeometry::GetCoverage(
    const Matrix& transform) const {
  if (points_.size() > 0) {
    // Doesn't use MakePointBounds as this isn't resilient to points that
    // all lie along the same axis.
    auto first = points_.begin();
    auto last = points_.end();
    auto left = first->x;
    auto top = first->y;
    auto right = first->x;
    auto bottom = first->y;
    for (auto it = first + 1; it < last; ++it) {
      left = std::min(left, it->x);
      top = std::min(top, it->y);
      right = std::max(right, it->x);
      bottom = std::max(bottom, it->y);
    }
    auto coverage = Rect::MakeLTRB(left - radius_, top - radius_,
                                   right + radius_, bottom + radius_);
    return coverage.TransformBounds(transform);
  }
  return std::nullopt;
}

}  // namespace impeller
