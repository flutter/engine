// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/rrect_shadow_contents.h"
#include <optional>

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/vertex_buffer_builder.h"
#include "impeller/tessellator/tessellator.h"

namespace impeller {

RRectShadowContents::RRectShadowContents() = default;

RRectShadowContents::~RRectShadowContents() = default;

void RRectShadowContents::SetRRect(std::optional<Rect> rect,
                                   Scalar corner_radius) {
  rect_ = rect;
  corner_radius_ = corner_radius;
}

void RRectShadowContents::SetSigma(Sigma sigma) {
  sigma_ = sigma;
}

void RRectShadowContents::SetColor(Color color) {
  color_ = color.Premultiply();
}

std::optional<Rect> RRectShadowContents::GetCoverage(
    const Entity& entity) const {
  if (!rect_.has_value()) {
    return std::nullopt;
  }

  Scalar radius = Radius{sigma_}.radius;

  auto ltrb = rect_->GetLTRB();
  Rect bounds = Rect::MakeLTRB(ltrb[0] - radius, ltrb[1] - radius,
                               ltrb[2] + radius, ltrb[3] + radius);
  return bounds.TransformBounds(entity.GetTransformation());
};

bool RRectShadowContents::Render(const ContentContext& renderer,
                                 const Entity& entity,
                                 RenderPass& pass) const {
  if (!rect_.has_value()) {
    return true;
  }

  using VS = RRectBlurPipeline::VertexShader;
  using FS = RRectBlurPipeline::FragmentShader;

  VertexBufferBuilder<VS::PerVertexData> vtx_builder;

  auto radius = Radius{sigma_}.radius;
  {
    auto left = -radius;
    auto top = -radius;
    auto right = rect_->size.width + radius;
    auto bottom = rect_->size.height + radius;

    vtx_builder.AddVertices({
        {Point(left, top)},
        {Point(right, top)},
        {Point(left, bottom)},
        {Point(left, bottom)},
        {Point(right, top)},
        {Point(right, bottom)},
    });
  }

  Command cmd;
  cmd.label = "Box Shadow";
  cmd.pipeline =
      renderer.GetRRectBlurPipeline(OptionsFromPassAndEntity(pass, entity));
  cmd.stencil_reference = entity.GetStencilDepth();

  cmd.primitive_type = PrimitiveType::kTriangle;
  cmd.BindVertices(vtx_builder.CreateVertexBuffer(pass.GetTransientsBuffer()));

  VS::VertInfo vert_info;
  vert_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                  entity.GetTransformation() *
                  Matrix::MakeTranslation({rect_->origin});
  VS::BindVertInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(vert_info));

  FS::FragInfo frag_info;
  frag_info.color = color_;
  frag_info.blur_radius = radius;
  frag_info.rect_size = Point(rect_->size);
  frag_info.corner_radius = corner_radius_;
  FS::BindFragInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(frag_info));

  if (!pass.AddCommand(std::move(cmd))) {
    return false;
  }

  return true;
}

}  // namespace impeller
