// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/display_list_atlas_geometry.h"

#include "impeller/entity/atlas_fill.frag.h"
#include "impeller/entity/atlas_fill.vert.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/position_color.vert.h"
#include "impeller/geometry/matrix.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/geometry/point.h"
#include "impeller/renderer/device_buffer.h"
#include "impeller/renderer/render_pass.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkRect.h"

namespace impeller {

static Color ToColor(const SkColor& color) {
  return {
      static_cast<Scalar>(SkColorGetR(color) / 255.0),  //
      static_cast<Scalar>(SkColorGetG(color) / 255.0),  //
      static_cast<Scalar>(SkColorGetB(color) / 255.0),  //
      static_cast<Scalar>(SkColorGetA(color) / 255.0)   //
  };
}

/////// Atlas Geometry ///////

// static
std::unique_ptr<AtlasGeometry> DLAtlasGeometry::MakeAtlas(
    const SkRSXform* xform,
    const SkRect* tex,
    const flutter::DlColor* colors,
    size_t count,
    std::optional<Rect> cull_rect) {
  return std::make_unique<DLAtlasGeometry>(xform, tex, colors, count,
                                           cull_rect);
}

DLAtlasGeometry::DLAtlasGeometry(const SkRSXform* xform,
                                 const SkRect* tex,
                                 const flutter::DlColor* colors,
                                 size_t count,
                                 std::optional<Rect> cull_rect)
    : colors_(colors), tex_(tex), count_(count), cull_rect_(cull_rect) {
  TransformPoints(xform);
}

DLAtlasGeometry::~DLAtlasGeometry() = default;

void DLAtlasGeometry::TransformPoints(const SkRSXform* xform) {
  transformed_coords_.reserve(count_);
  for (size_t i = 0; i < count_; i++) {
    auto sample_rect = tex_[i];
    auto form = xform[i];
    auto matrix = Matrix{
        // clang-format off
      form.fSCos, form.fSSin, 0, 0,
     -form.fSSin, form.fSCos, 0, 0,
      0,          0,          1, 0,
      form.fTx,   form.fTy,   0, 1
        // clang-format on
    };
    transformed_coords_.emplace_back(
        Rect::MakeSize(Size(sample_rect.width(), sample_rect.height()))
            .GetTransformedPoints(matrix));
  }
}

GeometryResult DLAtlasGeometry::GetPositionBuffer(
    const ContentContext& renderer,
    const Entity& entity,
    RenderPass& pass) {
  return {};
}

GeometryResult DLAtlasGeometry::GetPositionColorBuffer(
    const ContentContext& renderer,
    const Entity& entity,
    RenderPass& pass,
    ISize texture_size) {
  using VS = AtlasFillVertexShader;

  constexpr size_t indices[6] = {0, 1, 2, 1, 2, 3};
  constexpr Scalar width[6] = {0, 1, 0, 1, 0, 1};
  constexpr Scalar height[6] = {0, 0, 1, 0, 1, 1};

  VertexBufferBuilder<VS::PerVertexData> vertex_builder;
  vertex_builder.Reserve(count_ * 6);
  for (size_t i = 0; i < count_; i++) {
    auto color = colors_ == nullptr ? Color::Black() : ToColor(colors_[i]);
    auto points = transformed_coords_[i];
    auto raw_sample_rect = tex_[i];
    auto sample_rect =
        Rect::MakeLTRB(raw_sample_rect.fLeft, raw_sample_rect.fTop,
                       raw_sample_rect.fRight, raw_sample_rect.fBottom);

    for (size_t j = 0; j < 6; j++) {
      VS::PerVertexData data;
      data.position = points[indices[j]];
      data.texture_coords =
          (sample_rect.origin + Point(sample_rect.size.width * width[j],
                                      sample_rect.size.height * height[j])) /
          texture_size;
      data.color = color.Premultiply();
      vertex_builder.AppendVertex(data);
    }
  }

  return {
      .type = PrimitiveType::kTriangle,
      .vertex_buffer =
          vertex_builder.CreateVertexBuffer(pass.GetTransientsBuffer()),
      .prevent_overdraw = false,
  };
}

GeometryVertexType DLAtlasGeometry::GetVertexType() const {
  if (colors_ == nullptr) {
    return GeometryVertexType::kPosition;
  }
  return GeometryVertexType::kColor;
}

std::optional<Rect> DLAtlasGeometry::GetCoverage(
    const Matrix& transform) const {
  if (cull_rect_.has_value()) {
    return cull_rect_.value().TransformBounds(transform);
  }

  Rect bounding_box = {};
  for (size_t i = 0; i < count_; i++) {
    auto points = transformed_coords_[i];
    auto bounds = Rect::MakePointBounds({points.begin(), points.end()}).value();
    bounding_box = bounds.Union(bounding_box);
  }
  return bounding_box.TransformBounds(transform);
}

}  // namespace impeller
