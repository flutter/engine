// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <optional>
#include <utility>

#include "impeller/renderer/formats.h"
#include "impeller/renderer/sampler_library.h"
#include "impeller/renderer/vertex_buffer_builder.h"

#include "impeller/entity/atlas_fill.frag.h"
#include "impeller/entity/atlas_fill.vert.h"
#include "impeller/entity/contents/atlas_contents.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

AtlasContents::AtlasContents() = default;

AtlasContents::~AtlasContents() = default;

void AtlasContents::SetTexture(std::shared_ptr<Texture> texture) {
  texture_ = std::move(texture);
}

std::shared_ptr<Texture> AtlasContents::GetTexture() const {
  return texture_;
}

void AtlasContents::SetGeometry(std::unique_ptr<AtlasGeometry> geometry) {
  geometry_ = std::move(geometry);
}

void AtlasContents::SetAlpha(Scalar alpha) {
  alpha_ = alpha;
}

void AtlasContents::SetBlendMode(BlendMode blend_mode) {
  blend_mode_ = blend_mode;
}

std::optional<Rect> AtlasContents::GetCoverage(const Entity& entity) const {
  return geometry_->GetCoverage(entity.GetTransformation());
}

void AtlasContents::SetSamplerDescriptor(SamplerDescriptor desc) {
  sampler_descriptor_ = std::move(desc);
}

const SamplerDescriptor& AtlasContents::GetSamplerDescriptor() const {
  return sampler_descriptor_;
}

bool AtlasContents::Render(const ContentContext& renderer,
                           const Entity& entity,
                           RenderPass& pass) const {
  if (texture_ == nullptr) {
    return true;
  }

  using VS = AtlasFillVertexShader;
  using FS = AtlasFillFragmentShader;

  const auto texture_size = texture_->GetSize();
  if (texture_size.IsEmpty()) {
    return true;
  }
  auto geometry_result =
      geometry_->GetPositionColorBuffer(renderer, entity, pass, texture_size);
  auto& host_buffer = pass.GetTransientsBuffer();

  VS::VertInfo vert_info;
  vert_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                  entity.GetTransformation();

  FS::FragInfo frag_info;
  frag_info.texture_sampler_y_coord_scale = texture_->GetYCoordScale();
  frag_info.has_vertex_color =
      geometry_->GetVertexType() == GeometryVertexType::kColor;
  frag_info.alpha = alpha_;

  auto options = OptionsFromPassAndEntity(pass, entity);
  options.primitive_type = geometry_result.type;

  Command cmd;
  cmd.label = "DrawAtlas";
  cmd.pipeline = renderer.GetAtlasPipeline(options);
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(geometry_result.vertex_buffer);
  VS::BindVertInfo(cmd, host_buffer.EmplaceUniform(vert_info));
  FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));
  FS::BindTextureSampler(cmd, texture_,
                         renderer.GetContext()->GetSamplerLibrary()->GetSampler(
                             sampler_descriptor_));
  pass.AddCommand(std::move(cmd));

  return true;
}

}  // namespace impeller
