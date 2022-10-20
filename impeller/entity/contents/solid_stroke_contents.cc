// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "solid_stroke_contents.h"

#include <optional>

#include "impeller/entity/contents/clip_contents.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/solid_fill.frag.h"
#include "impeller/entity/solid_fill.vert.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

SolidStrokeContents::SolidStrokeContents(){};

SolidStrokeContents::~SolidStrokeContents() = default;

void SolidStrokeContents::SetColor(Color color) {
  color_ = color;
}

const Color& SolidStrokeContents::GetColor() const {
  return color_;
}

void SolidStrokeContents::SetGeometry(std::unique_ptr<Geometry> geometry) {
  geometry_ = std::move(geometry);
}

std::optional<Rect> SolidStrokeContents::GetCoverage(
    const Entity& entity) const {
  if (color_.IsTransparent()) {
    return std::nullopt;
  }
<<<<<<< HEAD
  return geometry_->GetCoverage(entity.GetTransformation());
=======

  auto path_bounds = path_.GetBoundingBox();
  if (!path_bounds.has_value()) {
    return std::nullopt;
  }
  auto path_coverage = path_bounds->TransformBounds(entity.GetTransformation());

  Scalar max_radius = 0.5;
  if (cap_ == Cap::kSquare) {
    max_radius = max_radius * kSqrt2;
  }
  if (join_ == Join::kMiter) {
    max_radius = std::max(max_radius, miter_limit_ * 0.5f);
  }
  Scalar determinant = entity.GetTransformation().GetDeterminant();
  if (determinant == 0) {
    return std::nullopt;
  }
  Scalar min_size = 1.0f / sqrt(std::abs(determinant));
  Vector2 max_radius_xy = entity.GetTransformation().TransformDirection(
      Vector2(max_radius, max_radius) * std::max(stroke_size_, min_size));

  return Rect(path_coverage.origin - max_radius_xy,
              Size(path_coverage.size.width + max_radius_xy.x * 2,
                   path_coverage.size.height + max_radius_xy.y * 2));
}

static VertexBuffer CreateSolidStrokeVertices(
    const Path& path,
    HostBuffer& buffer,
    Scalar stroke_width,
    const SolidStrokeContents::CapProc& cap_proc,
    const SolidStrokeContents::JoinProc& join_proc,
    Scalar miter_limit,
    Scalar tolerance) {
  using VS = SolidFillVertexShader;

  VertexBufferBuilder<VS::PerVertexData> vtx_builder;
  auto polyline = path.CreatePolyline();

  VS::PerVertexData vtx;

  // Offset state.
  Point offset;
  Point previous_offset;  // Used for computing joins.

  auto compute_offset = [&polyline, &offset, &previous_offset,
                         &stroke_width](size_t point_i) {
    previous_offset = offset;
    Point direction =
        (polyline.points[point_i] - polyline.points[point_i - 1]).Normalize();
    offset = Vector2{-direction.y, direction.x} * stroke_width * 0.5;
  };

  for (size_t contour_i = 0; contour_i < polyline.contours.size();
       contour_i++) {
    size_t contour_start_point_i, contour_end_point_i;
    std::tie(contour_start_point_i, contour_end_point_i) =
        polyline.GetContourPointBounds(contour_i);

    switch (contour_end_point_i - contour_start_point_i) {
      case 1: {
        Point p = polyline.points[contour_start_point_i];
        cap_proc(vtx_builder, p, {-stroke_width * 0.5f, 0}, tolerance);
        cap_proc(vtx_builder, p, {stroke_width * 0.5f, 0}, tolerance);
        continue;
      }
      case 0:
        continue;  // This contour has no renderable content.
      default:
        break;
    }

    // The first point's offset is always the same as the second point.
    compute_offset(contour_start_point_i + 1);
    const Point contour_first_offset = offset;

    if (contour_i > 0) {
      // This branch only executes when we've just finished drawing a contour
      // and are switching to a new one.
      // We're drawing a triangle strip, so we need to "pick up the pen" by
      // appending two vertices at the end of the previous contour and two
      // vertices at the start of the new contour (thus connecting the two
      // contours with two zero volume triangles, which will be discarded by the
      // rasterizer).
      vtx.position = polyline.points[contour_start_point_i - 1];
      // Append two vertices when "picking up" the pen so that the triangle
      // drawn when moving to the beginning of the new contour will have zero
      // volume.
      vtx_builder.AppendVertex(vtx);
      vtx_builder.AppendVertex(vtx);

      vtx.position = polyline.points[contour_start_point_i];
      // Append two vertices at the beginning of the new contour, which appends
      // two triangles of zero area.
      vtx_builder.AppendVertex(vtx);
      vtx_builder.AppendVertex(vtx);
    }

    // Generate start cap.
    if (!polyline.contours[contour_i].is_closed) {
      cap_proc(vtx_builder, polyline.points[contour_start_point_i], -offset,
               tolerance);
    }

    // Generate contour geometry.
    for (size_t point_i = contour_start_point_i; point_i < contour_end_point_i;
         point_i++) {
      if (point_i > contour_start_point_i) {
        // Generate line rect.
        vtx.position = polyline.points[point_i - 1] + offset;
        vtx_builder.AppendVertex(vtx);
        vtx.position = polyline.points[point_i - 1] - offset;
        vtx_builder.AppendVertex(vtx);
        vtx.position = polyline.points[point_i] + offset;
        vtx_builder.AppendVertex(vtx);
        vtx.position = polyline.points[point_i] - offset;
        vtx_builder.AppendVertex(vtx);

        if (point_i < contour_end_point_i - 1) {
          compute_offset(point_i + 1);

          // Generate join from the current line to the next line.
          join_proc(vtx_builder, polyline.points[point_i], previous_offset,
                    offset, miter_limit, tolerance);
        }
      }
    }

    // Generate end cap or join.
    if (!polyline.contours[contour_i].is_closed) {
      cap_proc(vtx_builder, polyline.points[contour_end_point_i - 1], offset,
               tolerance);
    } else {
      join_proc(vtx_builder, polyline.points[contour_start_point_i], offset,
                contour_first_offset, miter_limit, tolerance);
    }
  }

  return vtx_builder.CreateVertexBuffer(buffer);
>>>>>>> d0791965483dee6f98aa73f81fd481944a182c5e
}

bool SolidStrokeContents::Render(const ContentContext& renderer,
                                 const Entity& entity,
                                 RenderPass& pass) const {
  using VS = SolidFillVertexShader;
  using FS = SolidFillFragmentShader;
  VS::VertInfo vert_info;
  vert_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                  entity.GetTransformation();

  FS::FragInfo frag_info;
  frag_info.color = color_.Premultiply();

  Command cmd;
  cmd.label = "Solid Stroke";
  cmd.stencil_reference = entity.GetStencilDepth();

  auto geometry_result = geometry_->GetPositionBuffer(renderer, entity, pass);

  auto options = OptionsFromPassAndEntity(pass, entity);
  if (geometry_result.prevent_overdraw) {
    options.stencil_compare = CompareFunction::kEqual;
    options.stencil_operation = StencilOperation::kIncrementClamp;
  }
  cmd.pipeline = renderer.GetSolidFillPipeline(options);

  cmd.BindVertices(geometry_result.vertex_buffer);
  cmd.primitive_type = geometry_result.type;
  VS::BindVertInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(vert_info));
  FS::BindFragInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(frag_info));

  pass.AddCommand(cmd);

  if (geometry_result.prevent_overdraw) {
    return ClipRestoreContents().Render(renderer, entity, pass);
  }

  return true;
}

}  // namespace impeller
