// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iostream>

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
}

const std::vector<Color>& GradientGeneratorContents::GetColors() const {
  return colors_;
}

std::optional<Rect> GradientGeneratorContents::GetCoverage(
    const Entity& entity) const {
  auto minimum_delta = 1.0;
  for (size_t i = 1; i < stops_.size(); i++) {
    auto value = stops_[i] - stops_[i - 1];
    if (value < minimum_delta) {
      minimum_delta = value;
    }
  }
  // TODO: make sure we dont generate absurd texture sizes.
  auto scale = std::ceil(1.0 / minimum_delta);
  return Rect::MakeLTRB(0, 0, scale, 1);
}

bool GradientGeneratorContents::Render(const ContentContext& renderer,
                                       const Entity& entity,
                                       RenderPass& pass) const {
  using VS = GradientGeneratorPipeline::VertexShader;

  auto vertices_builder = VertexBufferBuilder<VS::PerVertexData>();
  auto minimum_delta = 1.0;
  for (size_t i = 1; i < stops_.size(); i++) {
    auto value = stops_[i] - stops_[i - 1];
    if (value < minimum_delta) {
      minimum_delta = value;
    }
  }
  auto scale = std::ceil(1.0 / minimum_delta);

  {
    assert(stops_.size() == colors_.size());
    for (size_t i = 1; i < stops_.size(); i++) {
      auto prev_stop = stops_[i-1] * scale;
      auto prev_color = colors_[i-1].Premultiply();
      auto stop = stops_[i] * scale;
      auto color = colors_[i].Premultiply();
      // Upper
      VS::PerVertexData vtx_1;
      vtx_1.position = Point(prev_stop, 0);
      vtx_1.color = prev_color;
      vertices_builder.AppendVertex(vtx_1);

      VS::PerVertexData vtx_2;
      vtx_2.position = Point(stop, 0);
      vtx_2.color = color;
      vertices_builder.AppendVertex(vtx_2);

      VS::PerVertexData vtx_3;
      vtx_3.position = Point(prev_stop, 1);
      vtx_3.color = prev_color;
      vertices_builder.AppendVertex(vtx_3);

      // Lower
      VS::PerVertexData vtx_4;
      vtx_4.position = Point(stop, 0);
      vtx_4.color = color;
      vertices_builder.AppendVertex(vtx_4);

      VS::PerVertexData vtx_5;
      vtx_5.position = Point(prev_stop, 1);
      vtx_5.color = prev_color;
      vertices_builder.AppendVertex(vtx_5);

      VS::PerVertexData vtx_6;
      vtx_6.position = Point(stop, 1);
      vtx_6.color = color;
      vertices_builder.AppendVertex(vtx_6);
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
