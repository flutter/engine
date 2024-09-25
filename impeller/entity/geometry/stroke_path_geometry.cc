// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/geometry/stroke_path_geometry.h"

#include "impeller/core/buffer_view.h"
#include "impeller/core/formats.h"
#include "impeller/entity/geometry/geometry.h"
#include "impeller/geometry/constants.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/geometry/path_component.h"
#include "impeller/geometry/separated_vector.h"

namespace impeller {
using VS = SolidFillVertexShader;

namespace {

class PositionWriter {
 public:
  void AppendVertex(const Point& point) { data_.push_back(point); }

  const std::vector<Point>& GetData() const { return data_; }

 private:
  std::vector<Point> data_ = {};
};

using CapProc = std::function<void(PositionWriter& vtx_builder,
                                   const Point& position,
                                   const Point& offset,
                                   Scalar scale,
                                   bool reverse)>;

using JoinProc = std::function<void(PositionWriter& vtx_builder,
                                    const Point& position,
                                    const Point& start_offset,
                                    const Point& end_offset,
                                    Scalar miter_limit,
                                    Scalar scale)>;

class StrokeGenerator {
 public:
  StrokeGenerator(const Path::Polyline& p_polyline,
                  const Scalar p_stroke_width,
                  const Scalar p_scaled_miter_limit,
                  const JoinProc& p_join_proc,
                  const CapProc& p_cap_proc,
                  const Scalar p_scale)
      : polyline(p_polyline),
        stroke_width(p_stroke_width),
        scaled_miter_limit(p_scaled_miter_limit),
        join_proc(p_join_proc),
        cap_proc(p_cap_proc),
        scale(p_scale) {}

  void Generate(PositionWriter& vtx_builder) {
    for (size_t contour_i = 0; contour_i < polyline.contours.size();
         contour_i++) {
      const Path::PolylineContour& contour = polyline.contours[contour_i];
      size_t contour_start_point_i, contour_end_point_i;
      std::tie(contour_start_point_i, contour_end_point_i) =
          polyline.GetContourPointBounds(contour_i);

      auto contour_delta = contour_end_point_i - contour_start_point_i;
      if (contour_delta == 1) {
        Point p = polyline.GetPoint(contour_start_point_i);
        cap_proc(vtx_builder, p, {-stroke_width * 0.5f, 0}, scale,
                 /*reverse=*/false);
        cap_proc(vtx_builder, p, {stroke_width * 0.5f, 0}, scale,
                 /*reverse=*/false);
        continue;
      } else if (contour_delta == 0) {
        continue;  // This contour has no renderable content.
      }

      previous_offset = offset;
      offset = ComputeOffset(contour_start_point_i, contour_start_point_i,
                             contour_end_point_i, contour);
      const Point contour_first_offset = offset.GetVector();

      if (contour_i > 0) {
        // This branch only executes when we've just finished drawing a contour
        // and are switching to a new one.
        // We're drawing a triangle strip, so we need to "pick up the pen" by
        // appending two vertices at the end of the previous contour and two
        // vertices at the start of the new contour (thus connecting the two
        // contours with two zero volume triangles, which will be discarded by
        // the rasterizer).
        vtx.position = polyline.GetPoint(contour_start_point_i - 1);
        // Append two vertices when "picking up" the pen so that the triangle
        // drawn when moving to the beginning of the new contour will have zero
        // volume.
        vtx_builder.AppendVertex(vtx.position);
        vtx_builder.AppendVertex(vtx.position);

        vtx.position = polyline.GetPoint(contour_start_point_i);
        // Append two vertices at the beginning of the new contour, which
        // appends  two triangles of zero area.
        vtx_builder.AppendVertex(vtx.position);
        vtx_builder.AppendVertex(vtx.position);
      }

      // Generate start cap.
      if (!polyline.contours[contour_i].is_closed) {
        Point cap_offset =
            Vector2(-contour.start_direction.y, contour.start_direction.x) *
            stroke_width * 0.5f;  // Counterclockwise normal
        cap_proc(vtx_builder, polyline.GetPoint(contour_start_point_i),
                 cap_offset, scale, /*reverse=*/true);
      }

      for (size_t contour_component_i = 0;
           contour_component_i < contour.components.size();
           contour_component_i++) {
        const Path::PolylineContour::Component& component =
            contour.components[contour_component_i];
        bool is_last_component =
            contour_component_i == contour.components.size() - 1;

        size_t component_start_index = component.component_start_index;
        size_t component_end_index =
            is_last_component ? contour_end_point_i - 1
                              : contour.components[contour_component_i + 1]
                                    .component_start_index;
        if (component.is_curve) {
          AddVerticesForCurveComponent(
              vtx_builder, component_start_index, component_end_index,
              contour_start_point_i, contour_end_point_i, contour);
        } else {
          AddVerticesForLinearComponent(
              vtx_builder, component_start_index, component_end_index,
              contour_start_point_i, contour_end_point_i, contour);
        }
      }

      // Generate end cap or join.
      if (!contour.is_closed) {
        auto cap_offset =
            Vector2(-contour.end_direction.y, contour.end_direction.x) *
            stroke_width * 0.5f;  // Clockwise normal
        cap_proc(vtx_builder, polyline.GetPoint(contour_end_point_i - 1),
                 cap_offset, scale, /*reverse=*/false);
      } else {
        join_proc(vtx_builder, polyline.GetPoint(contour_start_point_i),
                  offset.GetVector(), contour_first_offset, scaled_miter_limit,
                  scale);
      }
    }
  }

  /// Computes offset by calculating the direction from point_i - 1 to point_i
  /// if point_i is within `contour_start_point_i` and `contour_end_point_i`;
  /// Otherwise, it uses direction from contour.
  SeparatedVector2 ComputeOffset(const size_t point_i,
                                 const size_t contour_start_point_i,
                                 const size_t contour_end_point_i,
                                 const Path::PolylineContour& contour) const {
    Point direction;
    if (point_i >= contour_end_point_i) {
      direction = contour.end_direction;
    } else if (point_i <= contour_start_point_i) {
      direction = -contour.start_direction;
    } else {
      direction = (polyline.GetPoint(point_i) - polyline.GetPoint(point_i - 1))
                      .Normalize();
    }
    return SeparatedVector2(Vector2{-direction.y, direction.x},
                            stroke_width * 0.5f);
  }

  void AddVerticesForLinearComponent(PositionWriter& vtx_builder,
                                     const size_t component_start_index,
                                     const size_t component_end_index,
                                     const size_t contour_start_point_i,
                                     const size_t contour_end_point_i,
                                     const Path::PolylineContour& contour) {
    bool is_last_component = component_start_index ==
                             contour.components.back().component_start_index;

    for (size_t point_i = component_start_index; point_i < component_end_index;
         point_i++) {
      bool is_end_of_component = point_i == component_end_index - 1;

      Point offset_vector = offset.GetVector();

      vtx.position = polyline.GetPoint(point_i) + offset_vector;
      vtx_builder.AppendVertex(vtx.position);
      vtx.position = polyline.GetPoint(point_i) - offset_vector;
      vtx_builder.AppendVertex(vtx.position);

      // For line components, two additional points need to be appended
      // prior to appending a join connecting the next component.
      vtx.position = polyline.GetPoint(point_i + 1) + offset_vector;
      vtx_builder.AppendVertex(vtx.position);
      vtx.position = polyline.GetPoint(point_i + 1) - offset_vector;
      vtx_builder.AppendVertex(vtx.position);

      previous_offset = offset;
      offset = ComputeOffset(point_i + 2, contour_start_point_i,
                             contour_end_point_i, contour);
      if (!is_last_component && is_end_of_component) {
        // Generate join from the current line to the next line.
        join_proc(vtx_builder, polyline.GetPoint(point_i + 1),
                  previous_offset.GetVector(), offset.GetVector(),
                  scaled_miter_limit, scale);
      }
    }
  }

  void AddVerticesForCurveComponent(PositionWriter& vtx_builder,
                                    const size_t component_start_index,
                                    const size_t component_end_index,
                                    const size_t contour_start_point_i,
                                    const size_t contour_end_point_i,
                                    const Path::PolylineContour& contour) {
    bool is_last_component = component_start_index ==
                             contour.components.back().component_start_index;

    for (size_t point_i = component_start_index; point_i < component_end_index;
         point_i++) {
      bool is_end_of_component = point_i == component_end_index - 1;

      vtx.position = polyline.GetPoint(point_i) + offset.GetVector();
      vtx_builder.AppendVertex(vtx.position);
      vtx.position = polyline.GetPoint(point_i) - offset.GetVector();
      vtx_builder.AppendVertex(vtx.position);

      previous_offset = offset;
      offset = ComputeOffset(point_i + 2, contour_start_point_i,
                             contour_end_point_i, contour);

      // If the angle to the next segment is too sharp, round out the join.
      if (!is_end_of_component) {
        constexpr Scalar kAngleThreshold = 10 * kPi / 180;
        // `std::cosf` is not constexpr-able, unfortunately, so we have to bake
        // the alignment constant.
        constexpr Scalar kAlignmentThreshold =
            0.984807753012208;  // std::cosf(kThresholdAngle) -- 10 degrees

        // Use a cheap dot product to determine whether the angle is too sharp.
        if (previous_offset.GetAlignment(offset) < kAlignmentThreshold) {
          Scalar angle_total = previous_offset.AngleTo(offset).radians;
          Scalar angle = kAngleThreshold;

          // Bridge the large angle with additional geometry at
          // `kAngleThreshold` interval.
          while (angle < std::abs(angle_total)) {
            Scalar signed_angle = angle_total < 0 ? -angle : angle;
            Point offset =
                previous_offset.GetVector().Rotate(Radians(signed_angle));
            vtx.position = polyline.GetPoint(point_i) + offset;
            vtx_builder.AppendVertex(vtx.position);
            vtx.position = polyline.GetPoint(point_i) - offset;
            vtx_builder.AppendVertex(vtx.position);

            angle += kAngleThreshold;
          }
        }
      }

      // For curve components, the polyline is detailed enough such that
      // it can avoid worrying about joins altogether.
      if (is_end_of_component) {
        // Append two additional vertices to close off the component. If we're
        // on the _last_ component of the contour then we need to use the
        // contour's end direction.
        // `ComputeOffset` returns the contour's end direction when attempting
        // to grab offsets past `contour_end_point_i`, so just use `offset` when
        // we're on the last component.
        Point last_component_offset = is_last_component
                                          ? offset.GetVector()
                                          : previous_offset.GetVector();
        vtx.position = polyline.GetPoint(point_i + 1) + last_component_offset;
        vtx_builder.AppendVertex(vtx.position);
        vtx.position = polyline.GetPoint(point_i + 1) - last_component_offset;
        vtx_builder.AppendVertex(vtx.position);
        // Generate join from the current line to the next line.
        if (!is_last_component) {
          join_proc(vtx_builder, polyline.GetPoint(point_i + 1),
                    previous_offset.GetVector(), offset.GetVector(),
                    scaled_miter_limit, scale);
        }
      }
    }
  }

  const Path::Polyline& polyline;
  const Scalar stroke_width;
  const Scalar scaled_miter_limit;
  const JoinProc& join_proc;
  const CapProc& cap_proc;
  const Scalar scale;

  SeparatedVector2 previous_offset;
  SeparatedVector2 offset;
  SolidFillVertexShader::PerVertexData vtx;
};

void CreateButtCap(PositionWriter& vtx_builder,
                   const Point& position,
                   const Point& offset,
                   Scalar scale,
                   bool reverse) {
  Point orientation = offset * (reverse ? -1 : 1);
  VS::PerVertexData vtx;
  vtx.position = position + orientation;
  vtx_builder.AppendVertex(vtx.position);
  vtx.position = position - orientation;
  vtx_builder.AppendVertex(vtx.position);
}

void CreateRoundCap(PositionWriter& vtx_builder,
                    const Point& position,
                    const Point& offset,
                    Scalar scale,
                    bool reverse) {
  Point orientation = offset * (reverse ? -1 : 1);
  Point forward(offset.y, -offset.x);
  Point forward_normal = forward.Normalize();

  CubicPathComponent arc;
  if (reverse) {
    arc = CubicPathComponent(
        forward, forward + orientation * PathBuilder::kArcApproximationMagic,
        orientation + forward * PathBuilder::kArcApproximationMagic,
        orientation);
  } else {
    arc = CubicPathComponent(
        orientation,
        orientation + forward * PathBuilder::kArcApproximationMagic,
        forward + orientation * PathBuilder::kArcApproximationMagic, forward);
  }

  Point vtx = position + orientation;
  vtx_builder.AppendVertex(vtx);
  vtx = position - orientation;
  vtx_builder.AppendVertex(vtx);

  arc.ToLinearPathComponents(scale, [&vtx_builder, &vtx, forward_normal,
                                     position](const Point& point) {
    vtx = position + point;
    vtx_builder.AppendVertex(vtx);
    vtx = position + (-point).Reflect(forward_normal);
    vtx_builder.AppendVertex(vtx);
  });
}

void CreateSquareCap(PositionWriter& vtx_builder,
                     const Point& position,
                     const Point& offset,
                     Scalar scale,
                     bool reverse) {
  Point orientation = offset * (reverse ? -1 : 1);
  Point forward(offset.y, -offset.x);

  Point vtx = position + orientation;
  vtx_builder.AppendVertex(vtx);
  vtx = position - orientation;
  vtx_builder.AppendVertex(vtx);
  vtx = position + orientation + forward;
  vtx_builder.AppendVertex(vtx);
  vtx = position - orientation + forward;
  vtx_builder.AppendVertex(vtx);
}

Scalar CreateBevelAndGetDirection(PositionWriter& vtx_builder,
                                  const Point& position,
                                  const Point& start_offset,
                                  const Point& end_offset) {
  Point vtx = position;
  vtx_builder.AppendVertex(vtx);

  Scalar dir = start_offset.Cross(end_offset) > 0 ? -1 : 1;
  vtx = position + start_offset * dir;
  vtx_builder.AppendVertex(vtx);
  vtx = position + end_offset * dir;
  vtx_builder.AppendVertex(vtx);

  return dir;
}

void CreateMiterJoin(PositionWriter& vtx_builder,
                     const Point& position,
                     const Point& start_offset,
                     const Point& end_offset,
                     Scalar miter_limit,
                     Scalar scale) {
  Point start_normal = start_offset.Normalize();
  Point end_normal = end_offset.Normalize();

  // 1 for no joint (straight line), 0 for max joint (180 degrees).
  Scalar alignment = (start_normal.Dot(end_normal) + 1) / 2;
  if (ScalarNearlyEqual(alignment, 1)) {
    return;
  }

  Scalar direction = CreateBevelAndGetDirection(vtx_builder, position,
                                                start_offset, end_offset);

  Point miter_point = (((start_offset + end_offset) / 2) / alignment);
  if (miter_point.GetDistanceSquared({0, 0}) > miter_limit * miter_limit) {
    return;  // Convert to bevel when we exceed the miter limit.
  }

  // Outer miter point.
  VS::PerVertexData vtx;
  vtx.position = position + miter_point * direction;
  vtx_builder.AppendVertex(vtx.position);
}

void CreateRoundJoin(PositionWriter& vtx_builder,
                     const Point& position,
                     const Point& start_offset,
                     const Point& end_offset,
                     Scalar miter_limit,
                     Scalar scale) {
  Point start_normal = start_offset.Normalize();
  Point end_normal = end_offset.Normalize();

  // 0 for no joint (straight line), 1 for max joint (180 degrees).
  Scalar alignment = 1 - (start_normal.Dot(end_normal) + 1) / 2;
  if (ScalarNearlyEqual(alignment, 0)) {
    return;
  }

  Scalar direction = CreateBevelAndGetDirection(vtx_builder, position,
                                                start_offset, end_offset);

  Point middle =
      (start_offset + end_offset).Normalize() * start_offset.GetLength();
  Point middle_normal = middle.Normalize();

  Point middle_handle = middle + Point(-middle.y, middle.x) *
                                     PathBuilder::kArcApproximationMagic *
                                     alignment * direction;
  Point start_handle = start_offset + Point(start_offset.y, -start_offset.x) *
                                          PathBuilder::kArcApproximationMagic *
                                          alignment * direction;

  VS::PerVertexData vtx;
  CubicPathComponent(start_offset, start_handle, middle_handle, middle)
      .ToLinearPathComponents(scale, [&vtx_builder, direction, &vtx, position,
                                      middle_normal](const Point& point) {
        vtx.position = position + point * direction;
        vtx_builder.AppendVertex(vtx.position);
        vtx.position = position + (-point * direction).Reflect(middle_normal);
        vtx_builder.AppendVertex(vtx.position);
      });
}

void CreateBevelJoin(PositionWriter& vtx_builder,
                     const Point& position,
                     const Point& start_offset,
                     const Point& end_offset,
                     Scalar miter_limit,
                     Scalar scale) {
  CreateBevelAndGetDirection(vtx_builder, position, start_offset, end_offset);
}

void CreateSolidStrokeVertices(PositionWriter& vtx_builder,
                               const Path::Polyline& polyline,
                               Scalar stroke_width,
                               Scalar scaled_miter_limit,
                               const JoinProc& join_proc,
                               const CapProc& cap_proc,
                               Scalar scale) {
  StrokeGenerator stroke_generator(polyline, stroke_width, scaled_miter_limit,
                                   join_proc, cap_proc, scale);
  stroke_generator.Generate(vtx_builder);
}

// static
JoinProc GetJoinProc(Join stroke_join) {
  switch (stroke_join) {
    case Join::kBevel:
      return &CreateBevelJoin;
    case Join::kMiter:
      return &CreateMiterJoin;
    case Join::kRound:
      return &CreateRoundJoin;
  }
}

CapProc GetCapProc(Cap stroke_cap) {
  switch (stroke_cap) {
    case Cap::kButt:
      return &CreateButtCap;
    case Cap::kRound:
      return &CreateRoundCap;
    case Cap::kSquare:
      return &CreateSquareCap;
  }
}
}  // namespace

std::vector<Point> StrokePathGeometry::GenerateSolidStrokeVertices(
    const Path::Polyline& polyline,
    Scalar stroke_width,
    Scalar miter_limit,
    Join stroke_join,
    Cap stroke_cap,
    Scalar scale) {
  auto scaled_miter_limit = stroke_width * miter_limit * 0.5f;
  auto join_proc = GetJoinProc(stroke_join);
  auto cap_proc = GetCapProc(stroke_cap);
  StrokeGenerator stroke_generator(polyline, stroke_width, scaled_miter_limit,
                                   join_proc, cap_proc, scale);
  PositionWriter vtx_builder;
  stroke_generator.Generate(vtx_builder);
  return vtx_builder.GetData();
}

StrokePathGeometry::StrokePathGeometry(const Path& path,
                                       Scalar stroke_width,
                                       Scalar miter_limit,
                                       Cap stroke_cap,
                                       Join stroke_join)
    : path_(path),
      stroke_width_(stroke_width),
      miter_limit_(miter_limit),
      stroke_cap_(stroke_cap),
      stroke_join_(stroke_join) {}

StrokePathGeometry::~StrokePathGeometry() = default;

Scalar StrokePathGeometry::GetStrokeWidth() const {
  return stroke_width_;
}

Scalar StrokePathGeometry::GetMiterLimit() const {
  return miter_limit_;
}

Cap StrokePathGeometry::GetStrokeCap() const {
  return stroke_cap_;
}

Join StrokePathGeometry::GetStrokeJoin() const {
  return stroke_join_;
}

Scalar StrokePathGeometry::ComputeAlphaCoverage(const Matrix& transform) const {
  return Geometry::ComputeStrokeAlphaCoverage(transform, stroke_width_);
}

GeometryResult StrokePathGeometry::GetPositionBuffer(
    const ContentContext& renderer,
    const Entity& entity,
    RenderPass& pass) const {
  if (stroke_width_ < 0.0) {
    return {};
  }
  Scalar max_basis = entity.GetTransform().GetMaxBasisLengthXY();
  if (max_basis == 0) {
    return {};
  }

  Scalar min_size = kMinStrokeSize / max_basis;
  Scalar stroke_width = std::max(stroke_width_, min_size);
  bool is_hairline = stroke_width_ <= min_size;
  auto& host_buffer = renderer.GetTransientsBuffer();

  // This is a harline stroke and can be drawn directly with line primitives,
  // which avoids extra tessellation work, cap/joins, and overdraw prevention.
  if (is_hairline && path_.IsSingleContour()) {
    // TODO(jonahwilliams): this could apply to multi contour paths if we add
    // support for primitive restart.
    auto vertex_buffer = renderer.GetTessellator()->TessellateConvex(
        path_, host_buffer, max_basis, /*line_strip=*/true);
    return GeometryResult{
        .type = PrimitiveType::kLineStrip,
        .vertex_buffer = vertex_buffer,
        .transform = entity.GetShaderTransform(pass),
    };
  }
  // For harline strokes that must be tessellated, switch to the cheaper
  // cap and joins since they will be barely noticable at pixel scale.
  Join join = stroke_join_;
  Cap cap = stroke_cap_;
  if (is_hairline) {
    join = Join::kBevel;
    cap = Cap::kButt;
  }

  PositionWriter position_writer;
  auto polyline =
      renderer.GetTessellator()->CreateTempPolyline(path_, max_basis);
  CreateSolidStrokeVertices(position_writer, polyline, stroke_width,
                            miter_limit_ * stroke_width_ * 0.5f,
                            GetJoinProc(join), GetCapProc(cap), max_basis);

  BufferView buffer_view = host_buffer.Emplace(
      position_writer.GetData().data(),
      position_writer.GetData().size() * sizeof(Point), alignof(Point));

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer =
          {
              .vertex_buffer = buffer_view,
              .vertex_count = position_writer.GetData().size(),
              .index_type = IndexType::kNone,
          },
      .transform = entity.GetShaderTransform(pass),
      .mode = GeometryResult::Mode::kPreventOverdraw};
}

GeometryResult::Mode StrokePathGeometry::GetResultMode() const {
  return GeometryResult::Mode::kPreventOverdraw;
}

std::optional<Rect> StrokePathGeometry::GetCoverage(
    const Matrix& transform) const {
  auto path_bounds = path_.GetBoundingBox();
  if (!path_bounds.has_value()) {
    return std::nullopt;
  }

  Scalar max_radius = 0.5;
  if (stroke_cap_ == Cap::kSquare) {
    max_radius = max_radius * kSqrt2;
  }
  if (stroke_join_ == Join::kMiter) {
    max_radius = std::max(max_radius, miter_limit_ * 0.5f);
  }
  Scalar max_basis = transform.GetMaxBasisLengthXY();
  if (max_basis == 0) {
    return {};
  }
  // Use the most conervative coverage setting.
  Scalar min_size = kMinStrokeSize / max_basis;
  max_radius *= std::max(stroke_width_, min_size);
  return path_bounds->Expand(max_radius).TransformBounds(transform);
}

}  // namespace impeller
