// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/box_shadow_contents.h"
#include <optional>

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/tessellator/tessellator.h"

namespace impeller {

BoxShadowContents::BoxShadowContents() = default;

BoxShadowContents::~BoxShadowContents() = default;

void BoxShadowContents::SetRect(std::optional<Rect> rect) {
  rect_ = rect;
}

void BoxShadowContents::SetSigma(FilterContents::Sigma sigma) {
  sigma_ = sigma;
}

void BoxShadowContents::SetColor(Color color) {
  color_ = color.Premultiply();
}

std::optional<Rect> BoxShadowContents::GetCoverage(const Entity& entity) const {
  if (!rect_.has_value()) {
    return std::nullopt;
  }

  Scalar border = FilterContents::Radius{sigma_}.radius;
  Rect bounds =
      Rect::MakeLTRB(rect_->GetLeft() - border, rect_->GetTop() - border,
                     rect_->GetRight() + border, rect_->GetBottom() + border);
  return bounds.TransformBounds(entity.GetTransformation());
};

/// The box shadow consists of 9 quads built from 16 vertices; 4 inner vertices
/// and 12 outset border vertices.
///
///   04---05----06---07
///   |    |      |    |
///   15---00----01---08
///   |    |      |    |
///   |    |      |    |
///   14---03----02---09
///   |    |      |    |
///   13---12----11---10
///
const static size_t kIndexCount = 9 * 6;
const static uint16_t kIndices[kIndexCount] = {
    4,  5, 0,  4,  0,  15,  // Top left
    5,  6, 1,  5,  1,  0,   // Top middle
    6,  7, 8,  6,  8,  1,   // Top right
    15, 0, 3,  15, 3,  14,  // Middle left
    0,  1, 2,  1,  2,  3,   // Middle middle
    1,  8, 9,  1,  9,  2,   // Middle right
    14, 3, 12, 14, 12, 13,  // Bottom left
    3,  2, 11, 3,  11, 12,  // Bottom middle
    2,  9, 10, 2,  10, 11,  // Bottom right
};

bool BoxShadowContents::Render(const ContentContext& renderer,
                               const Entity& entity,
                               RenderPass& pass) const {
  if (!rect_.has_value()) {
    return true;
  }

  if (color_.IsTransparent()) {
    return true;
  }

  using VS = BoxShadowPipeline::VertexShader;
  using FS = BoxShadowPipeline::FragmentShader;

  auto radius = FilterContents::Radius{sigma_}.radius;
  auto box = rect_->GetPoints();
  VS::PerVertexData vertices[16] = {
      // Middle box
      {box[0], Point()},
      {box[1], Point()},
      {box[3], Point()},
      {box[2], Point()},

      // Outset border
      {box[0] + Point(-radius, -radius), Point(1, 1)},
      {box[0] + Point(0, -radius), Point(0, 1)},
      {box[1] + Point(0, -radius), Point(0, 1)},

      {box[1] + Point(radius, -radius), Point(1, 1)},
      {box[1] + Point(radius, 0), Point(1, 0)},
      {box[2] + Point(radius, 0), Point(1, 0)},

      {box[3] + Point(radius, radius), Point(1, 1)},
      {box[3] + Point(0, radius), Point(0, 1)},
      {box[2] + Point(0, radius), Point(0, 1)},

      {box[2] + Point(-radius, radius), Point(1, 1)},
      {box[2] + Point(-radius, 0), Point(1, 0)},
      {box[0] + Point(-radius, 0), Point(1, 0)},
  };

  VertexBuffer vertex_buffer = {
      .vertex_buffer = pass.GetTransientsBuffer().Emplace(
          &vertices, sizeof(vertices), alignof(VS::PerVertexData)),
      .index_buffer = pass.GetTransientsBuffer().Emplace(
          &kIndices, sizeof(kIndices), alignof(uint16_t)),
      .index_count = kIndexCount,
      .index_type = IndexType::k16bit,
  };

  Command cmd;
  cmd.label = "Box Shadow";
  cmd.pipeline =
      renderer.GetBoxShadowPipeline(OptionsFromPassAndEntity(pass, entity));
  cmd.stencil_reference = entity.GetStencilDepth();

  cmd.primitive_type = PrimitiveType::kTriangle;
  cmd.BindVertices(vertex_buffer);

  VS::VertexInfo vertex_info;
  vertex_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                    entity.GetTransformation();
  VS::BindVertexInfo(cmd,
                     pass.GetTransientsBuffer().EmplaceUniform(vertex_info));

  FS::FragmentInfo fragment_info;
  fragment_info.color = color_;
  FS::BindFragmentInfo(
      cmd, pass.GetTransientsBuffer().EmplaceUniform(fragment_info));

  if (!pass.AddCommand(std::move(cmd))) {
    return false;
  }

  return true;
}

}  // namespace impeller
