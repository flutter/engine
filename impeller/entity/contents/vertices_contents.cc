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

VerticesContents::VerticesContents() = default;

VerticesContents::~VerticesContents() = default;

void VerticesContents::SetVertices(const flutter::DlVertices* vertices) {
  vertices_ = std::move(vertices);
}

std::optional<Rect> TextureContents::GetCoverage(const Entity& entity) const {
  return vertices_.bounds();
};

bool TextureContents::Render(const ContentContext& renderer,
                             const Entity& entity,
                             RenderPass& pass) const {

  using VS = VerticesVertexShader;
  using FS = VerticesFragmentShader;

  const auto coverage_rect = vertices_.bounds();

  if (!coverage_rect.has_value()) {
    return true;
  }

  if (coverage_rect->size.IsEmpty()) {
    return true;
  }


  VertexBufferBuilder<VS::PerVertexData> vertex_builder;
  for (int i = 0; i < vertices_.Size(); i++) {
      vertex_builder.AppendVertex(vertices_.vertices()[i]);
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
      renderer.GetTexturePipeline(OptionsFromPassAndEntity(pass, entity));
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(vertex_builder.CreateVertexBuffer(host_buffer));
  VS::BindFrameInfo(cmd, host_buffer.EmplaceUniform(frame_info));
  pass.AddCommand(std::move(cmd));

  return true;
}

}  // namespace impeller
