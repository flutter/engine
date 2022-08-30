// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "gradient_generator_contents.h"

#include "flutter/fml/logging.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/tessellator/tessellator.h"

namespace impeller {

GradientGeneratorContents::GradientGeneratorContents() = default;

GradientGeneratorContents::~GradientGeneratorContents() = default;

void GradientGeneratorContents::SetColors(std::vector<Color> colors) {
  colors_ = std::move(colors);
}

void GradientGeneratorContents::SetStops(std::vector<Scalar> stops) {
  stops_ = std::move(stops);
  auto minimum_delta = 1.0;
  for (size_t i = 1; i < stops_.size(); i++) {
    auto value = stops_[i] - stops_[i - 1];
    if (value < minimum_delta) {
      minimum_delta = value;
    }
  }
  // Avoid creating textures that are absurdly large due to stops that are
  // very close together.
  scale_ = std::min(std::ceil(1.0 / minimum_delta), 1024.0);
}

const std::vector<Color>& GradientGeneratorContents::GetColors() const {
  return colors_;
}

std::optional<Rect> GradientGeneratorContents::GetCoverage(
    const Entity& entity) const {
  return Rect::MakeLTRB(0, 0, scale_, 1);
}

bool GradientGeneratorContents::Render(const ContentContext& renderer,
                                       const Entity& entity,
                                       RenderPass& pass) const {
  using VS = GradientGeneratorPipeline::VertexShader;

  auto vertices_builder = VertexBufferBuilder<VS::PerVertexData>();
  {
    constexpr size_t indexing[6] = {0, 1, 0, 1, 0, 1};
    constexpr size_t y[6] = {0, 0, 1, 0, 1, 1};
    for (size_t i = 1; i < stops_.size(); i++) {
      auto prev_stop = stops_[i - 1] * scale_;
      auto prev_color = colors_[i - 1].Premultiply();
      auto stop = stops_[i] * scale_;
      auto color = colors_[i].Premultiply();
      for (size_t j = 0; j < 6; j++) {
        VS::PerVertexData vtx;
        vtx.position = Point(indexing[j] ? stop : prev_stop, y[j]);
        vtx.color = indexing[j] ? color : prev_color;
        vertices_builder.AppendVertex(vtx);
      }
    }
  }
  VS::FrameInfo frame_info;
  frame_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransformation();

  Command cmd;
  cmd.label = "GradientGenerator";
  cmd.pipeline = renderer.GetGradientGeneratorPipeline(
      OptionsFromPassAndEntity(pass, entity));
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(
      vertices_builder.CreateVertexBuffer(pass.GetTransientsBuffer()));
  cmd.primitive_type = PrimitiveType::kTriangle;
  VS::BindFrameInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(frame_info));
  return pass.AddCommand(std::move(cmd));
}

}  // namespace impeller
