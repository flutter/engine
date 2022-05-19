// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vertices_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"
#include "impeller/entity/vertices.frag.h"
#include "impeller/entity/vertices.vert.h"

namespace impeller {

VerticesContents::VerticesContents(Vertices vertices) : vertices_(vertices) {};

VerticesContents::~VerticesContents() = default;

std::optional<Rect> VerticesContents::GetCoverage(const Entity& entity) const {
  return vertices_.GetBoundingBox();
};

bool VerticesContents::Render(const ContentContext& renderer,
                              const Entity& entity,
                              RenderPass& pass) const {

  using VS = VerticesVertexShader;

  const auto coverage_rect = vertices_.GetBoundingBox();

  if (!coverage_rect.has_value()) {
    return true;
  }

  if (coverage_rect->size.IsEmpty()) {
    return true;
  }

  VertexBufferBuilder<VS::PerVertexData> vertex_builder;
  std::vector<Point> points = vertices_.get_points();
  std::vector<uint16_t> indexes = vertices_.get_indexes();

  if (indexes.size() == 0) {
    for (uint i = 0; i < points.size(); i += 1) {
      VS::PerVertexData data;
      data.vertices = points[i];
      vertex_builder.AppendVertex(data);
    }
  } else {
    for (uint i = 0; i < indexes.size(); i += 1) {
      VS::PerVertexData data;
      data.vertices = points[indexes[i]];
      vertex_builder.AppendVertex(data);
    }
  }

  if (!vertex_builder.HasVertices()) {
    return true;
  }

  auto& host_buffer = pass.GetTransientsBuffer();

  VS::FrameInfo frame_info;
  frame_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransformation();

  Command cmd;
  cmd.label = "Vertices";
  cmd.pipeline =
      renderer.GetVerticesPipeline(OptionsFromPassAndEntity(pass, entity));
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(vertex_builder.CreateVertexBuffer(host_buffer));
  VS::BindFrameInfo(cmd, host_buffer.EmplaceUniform(frame_info));
  pass.AddCommand(std::move(cmd));

  return true;
}

}  // namespace impeller
