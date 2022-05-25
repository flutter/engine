// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vertices_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/vertices.frag.h"
#include "impeller/entity/vertices.vert.h"
#include "impeller/geometry/color.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"

namespace impeller {

VerticesContents::VerticesContents(Vertices vertices)
    : vertices_(std::move(vertices)){};

VerticesContents::~VerticesContents() = default;

std::optional<Rect> VerticesContents::GetCoverage(const Entity& entity) const {
  return vertices_.GetTransformedBoundingBox(entity.GetTransformation());
};

void VerticesContents::SetColor(Color color) {
  color_ = color.Premultiply();
}

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
  std::vector<Point> points = vertices_.GetPoints();
  std::vector<uint16_t> indices = vertices_.GetIndices();
  if (points.size() == 0) {
    return true;
  }

  // TODO: colors are currently unused, must be blended with
  // paint color based on provided blend mode.
  VertexMode mode = vertices_.GetMode();

  size_t total_vtx_bytes = points.size() * 8;
  size_t total_idx_bytes = indices.size() * 2;
  auto buffer = renderer.GetContext()->GetTransientsAllocator()->CreateBuffer(
      impeller::StorageMode::kHostVisible, total_vtx_bytes + total_idx_bytes);

  size_t vertex_buffer_offset = 0;
  size_t index_buffer_offset = total_vtx_bytes;

  if (!buffer->CopyHostBuffer(reinterpret_cast<uint8_t*>(points.data()),
                              impeller::Range{0, total_vtx_bytes},
                              vertex_buffer_offset)) {
    // Do Something
  }
  if (!buffer->CopyHostBuffer(reinterpret_cast<uint8_t*>(indices.data()),
                              impeller::Range{0, total_idx_bytes},
                              index_buffer_offset)) {
    // Do something
  }

  PrimitiveType primitiveType;
  switch (mode) {
    case VertexMode::kTriangle:
      primitiveType = PrimitiveType::kTriangle;
      break;
    case VertexMode::kTriangleStrip:
      primitiveType = PrimitiveType::kTriangleStrip;
      break;
  }

  impeller::VertexBuffer vertex_buffer;
  vertex_buffer.vertex_buffer = {.buffer = buffer,
                                 .range = impeller::Range(0, total_vtx_bytes)};
  vertex_buffer.index_buffer = {
      .buffer = buffer,
      .range = impeller::Range(index_buffer_offset,
                               total_idx_bytes + index_buffer_offset)};
  vertex_buffer.index_count = indices.size();
  vertex_buffer.index_type = impeller::IndexType::k16bit;

  auto& host_buffer = pass.GetTransientsBuffer();

  VS::FrameInfo frame_info;
  frame_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransformation();

  Command cmd;
  cmd.label = "Vertices";
  cmd.pipeline =
      renderer.GetVerticesPipeline(OptionsFromPassAndEntity(pass, entity));
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.primitive_type = primitiveType;
  cmd.BindVertices(vertex_buffer);
  cmd.base_vertex = 0;
  VS::BindFrameInfo(cmd, host_buffer.EmplaceUniform(frame_info));
  pass.AddCommand(std::move(cmd));

  return true;
}

}  // namespace impeller
