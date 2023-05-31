// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/geometry/geometry.h"

#include "impeller/entity/geometry/cover_geometry.h"
#include "impeller/entity/geometry/fill_path_geometry.h"
#include "impeller/entity/geometry/point_field_geometry.h"
#include "impeller/entity/geometry/rect_geometry.h"
#include "impeller/entity/geometry/stroke_path_geometry.h"

namespace impeller {

/// Given a convex polyline, create a triangle fan structure.
std::pair<std::vector<Point>, std::vector<uint16_t>> TessellateConvex(
    Path::Polyline polyline) {
  std::vector<Point> output;
  std::vector<uint16_t> indices;

  for (auto j = 0u; j < polyline.contours.size(); j++) {
    auto [start, end] = polyline.GetContourPointBounds(j);
    auto center = polyline.points[start];

    // Some polygons will not self close and an additional triangle
    // must be inserted, others will self close and we need to avoid
    // inserting an extra triangle.
    if (polyline.points[end - 1] == polyline.points[start]) {
      end--;
    }
    output.emplace_back(center);
    output.emplace_back(polyline.points[start + 1]);

    for (auto i = start + 2; i < end; i++) {
      const auto& point_b = polyline.points[i];
      output.emplace_back(point_b);

      indices.emplace_back(0);
      indices.emplace_back(i - 1);
      indices.emplace_back(i);
    }
  }
  return std::make_pair(output, indices);
}

static bool IsConvex(const Point& a,
                     const Point& b,
                     const Point& c,
                     Scalar scale) {
  // The vectors ab and bc are convex if the angle between them is less
  // than 180 degrees. We can determine the proper angle computation by
  // determining which direction the triangle is facing w.r.t the axis
  // that the polygon is monotone in. Consider:
  //
  //
  //       B -- C
  //      /
  //     A
  //
  // Given the following points for A, B, C and that we are moving left to
  // right creating triangles. Visually, we can tell that this angle is
  // "convex". This can be determined computationally by realizing that the
  // direction of AB and BC is consistently moving left to right (it doesn't
  // change direction). In contrast:
  //
  //      C
  //      |
  //      B
  //     /
  //    A
  //
  // We can see in the above diagram that the direction changes moving from
  // AB to BC. Thus the angle is not convex.
  //
  // This left/right side determination is computed via comparing the cross
  // product of AB and AC. if resulting vector has a positive Z, then it lies
  // inside the polygon (when moving left to right) and outside the polygon
  // when moving right to left.
  auto ba = Point(a.x - b.x, a.y - b.y);
  auto bc = Point(c.x - b.x, c.y - b.y);
  return (ba.Cross(bc) * scale) > 0;
}

std::optional<std::vector<Point>> VerifyMonotone(Path::Polyline polyline,
                                                 bool xaxis) {
  if (polyline.contours.size() > 1) {
    return std::nullopt;
  }

  auto [start, end] = polyline.GetContourPointBounds(0);

  FML_DCHECK(start == 0u);  // Assumes single countour.
  size_t vertex_count = end;

  // To determine if this is horizontally monotone, first find the largest
  // and the smallest vertices in terms of the X coordinates. These vertices
  // split the polygon into an "upper" and "lower" segment. Note, that we
  // need to make an adjustment to support line segments that lie exactly
  // along the X axis. in this case, if the X value is nearly identical and
  // it comes from the prior index, we permit.
  size_t min_x_index = 0u;
  size_t max_x_index = 0u;
  Scalar min_x;
  Scalar max_x;
  if (xaxis) {
    min_x = polyline.points[min_x_index].x;
    max_x = polyline.points[max_x_index].x;
  } else {
    min_x = polyline.points[min_x_index].y;
    max_x = polyline.points[max_x_index].y;
  }
  for (auto i = 1u; i < end; i++) {
    auto x = xaxis ? polyline.points[i].x : polyline.points[i].y;
    if (x < min_x) {
      min_x_index = i;
      min_x = x;
    } else if (x > max_x) {
      max_x_index = i;
      max_x = x;
    }
  }

  // Next, We must verify that the xcoordinates are non-decreasing in left to
  // right order. This is done in order for the upper chain and in reverse order
  // on the lower chain.

  // Verify upper chain.
  Scalar prev_x = min_x;
  for (auto i = 1u; i < vertex_count; i++) {
    auto j = (min_x_index + i) % vertex_count;
    if (j == max_x_index) {
      break;
    }
    auto x = xaxis ? polyline.points[j].x : polyline.points[j].y;
    if (x < prev_x) {
      return std::nullopt;
    }
    prev_x = x;
  }

  // Verify lower chain.
  prev_x = max_x;
  for (auto i = 1u; i < vertex_count; i++) {
    auto j = (max_x_index + i) % vertex_count;
    if (j == min_x_index) {
      break;
    }
    auto x = xaxis ? polyline.points[j].x : polyline.points[j].y;
    if (x > prev_x) {
      return std::nullopt;
    }
    prev_x = x;
  }

  // At this point, we've verifed that the polygon is X or Y monotone.
  // This allows for a simple triangulation where we walk one of the chains
  // and greedly add triangles. This is done via a simplified ear clip.
  //
  // We traverse each monotone chain, and add triangles from a working set
  // as long as the angle is convex. The working set begins with the first
  // two vertices.

  std::vector<Point> result_set;
  std::vector<Point> candidate_set;
  auto next_index_from_min = (min_x_index + 1) % vertex_count;
  candidate_set.push_back(polyline.points[min_x_index]);
  candidate_set.push_back(polyline.points[next_index_from_min]);
  auto i = (min_x_index + 2) % vertex_count;

  while (i != min_x_index) {
    auto current = polyline.points[i];
    if (IsConvex(candidate_set[candidate_set.size() - 2],
                 candidate_set[candidate_set.size() - 1], current, -1.0)) {
      result_set.push_back(candidate_set[candidate_set.size() - 2]);
      result_set.push_back(candidate_set[candidate_set.size() - 1]);
      result_set.push_back(current);
      candidate_set.pop_back();
      if (candidate_set.size() < 1) {
        i = (i + 1) % vertex_count;
      }
    } else {
      candidate_set.push_back(current);
      i = (i + 1) % vertex_count;
    }
  }

  return result_set;
}

VertexBufferBuilder<TextureFillVertexShader::PerVertexData>
ComputeUVGeometryCPU(
    VertexBufferBuilder<SolidFillVertexShader::PerVertexData>& input,
    Point texture_origin,
    Size texture_coverage,
    Matrix effect_transform) {
  VertexBufferBuilder<TextureFillVertexShader::PerVertexData> vertex_builder;
  vertex_builder.Reserve(input.GetVertexCount());
  input.IterateVertices(
      [&vertex_builder, &texture_coverage, &effect_transform,
       &texture_origin](SolidFillVertexShader::PerVertexData old_vtx) {
        TextureFillVertexShader::PerVertexData data;
        data.position = old_vtx.position;
        auto coverage_coords =
            (old_vtx.position - texture_origin) / texture_coverage;
        data.texture_coords = effect_transform * coverage_coords;
        vertex_builder.AppendVertex(data);
      });
  return vertex_builder;
}

GeometryResult ComputeUVGeometryForRect(Rect source_rect,
                                        Rect texture_coverage,
                                        Matrix effect_transform,
                                        const ContentContext& renderer,
                                        const Entity& entity,
                                        RenderPass& pass) {
  auto& host_buffer = pass.GetTransientsBuffer();

  std::vector<Point> data(8);
  auto points = source_rect.GetPoints();
  for (auto i = 0u, j = 0u; i < 8; i += 2, j++) {
    data[i] = points[j];
    data[i + 1] = effect_transform * ((points[j] - texture_coverage.origin) /
                                      texture_coverage.size);
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

Geometry::Geometry() = default;

Geometry::~Geometry() = default;

GeometryResult Geometry::GetPositionUVBuffer(Rect texture_coverage,
                                             Matrix effect_transform,
                                             const ContentContext& renderer,
                                             const Entity& entity,
                                             RenderPass& pass) {
  return {};
}

std::unique_ptr<Geometry> Geometry::MakeFillPath(const Path& path) {
  return std::make_unique<FillPathGeometry>(path);
}

std::unique_ptr<Geometry> Geometry::MakePointField(std::vector<Point> points,
                                                   Scalar radius,
                                                   bool round) {
  return std::make_unique<PointFieldGeometry>(std::move(points), radius, round);
}

std::unique_ptr<Geometry> Geometry::MakeStrokePath(const Path& path,
                                                   Scalar stroke_width,
                                                   Scalar miter_limit,
                                                   Cap stroke_cap,
                                                   Join stroke_join) {
  // Skia behaves like this.
  if (miter_limit < 0) {
    miter_limit = 4.0;
  }
  return std::make_unique<StrokePathGeometry>(path, stroke_width, miter_limit,
                                              stroke_cap, stroke_join);
}

std::unique_ptr<Geometry> Geometry::MakeCover() {
  return std::make_unique<CoverGeometry>();
}

std::unique_ptr<Geometry> Geometry::MakeRect(Rect rect) {
  return std::make_unique<RectGeometry>(rect);
}

}  // namespace impeller
