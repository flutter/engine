// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vertices_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/position.vert.h"
#include "impeller/entity/position_color.vert.h"
#include "impeller/entity/position_uv.vert.h"
#include "impeller/entity/vertices.frag.h"
#include "impeller/geometry/color.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"
#include "impeller/renderer/vertex_buffer.h"

namespace impeller {

VerticesContents::VerticesContents() = default;

VerticesContents::~VerticesContents() = default;

std::optional<Rect> VerticesContents::GetCoverage(const Entity& entity) const {
  return geometry_->GetCoverage(entity.GetTransformation());
};

void VerticesContents::SetGeometry(std::unique_ptr<VerticesGeometry> geometry) {
  geometry_ = std::move(geometry);
}

void VerticesContents::SetAlpha(Scalar alpha) {
  alpha_ = alpha;
}

void VerticesContents::SetBlendMode(BlendMode blend_mode) {
  blend_mode_ = blend_mode;
}

void VerticesContents::SetSrcContents(std::shared_ptr<Contents> src_contents) {
  src_contents_ = std::move(src_contents);
}

bool VerticesContents::Render(const ContentContext& renderer,
                              const Entity& entity,
                              RenderPass& pass) const {
  if (blend_mode_ == BlendMode::kDestination) {
    return RenderDestination(renderer, entity, pass);
  }
  if (blend_mode_ == BlendMode::kSource ||
      (geometry_->HasTextureCoordinates() && !geometry_->HasVertexColors())) {
    return RenderSource(renderer, entity, pass);
  }

  using VS = AtlasBlendSrcOverPipeline::VertexShader;
  using FS = AtlasBlendSrcOverPipeline::FragmentShader;

  auto& host_buffer = pass.GetTransientsBuffer();

  Command cmd;
  cmd.label = "Vertices";
  cmd.stencil_reference = entity.GetStencilDepth();

  auto opts = OptionsFromPassAndEntity(pass, entity);
  auto geometry_result =
      geometry_->GetPositionUVColorBuffer(renderer, entity, pass);
  cmd.BindVertices(geometry_result.vertex_buffer);
  opts.primitive_type = geometry_result.type;

  switch (blend_mode_) {
    case BlendMode::kClear:
    case BlendMode::kSource:
    case BlendMode::kDestination:
      // All handled above.
      return true;
    case BlendMode::kSourceOver:
      cmd.pipeline = renderer.GetAtlasBlendSrcOverPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kDestinationOver:
      cmd.pipeline = renderer.GetAtlasBlendDstOverPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kSourceIn:
      cmd.pipeline = renderer.GetAtlasBlendSrcInPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kDestinationIn:
      cmd.pipeline = renderer.GetAtlasBlendDstInPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kSourceOut:
      cmd.pipeline = renderer.GetAtlasBlendSrcOutPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kDestinationOut:
      cmd.pipeline = renderer.GetAtlasBlendDstOutPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kSourceATop:
      cmd.pipeline = renderer.GetAtlasBlendSrcATopPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kDestinationATop:
      cmd.pipeline = renderer.GetAtlasBlendDstATopPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kXor:
      cmd.pipeline = renderer.GetAtlasBlendXorPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kPlus:
      cmd.pipeline = renderer.GetAtlasBlendPlusPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kModulate:
      cmd.pipeline = renderer.GetAtlasBlendModulatePipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    // Advanced
    case BlendMode::kScreen:
      cmd.pipeline = renderer.GetAtlasBlendScreenPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kOverlay:
      cmd.pipeline = renderer.GetAtlasBlendOverlayPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kDarken:
      cmd.pipeline = renderer.GetAtlasBlendDarkenPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kLighten:
      cmd.pipeline = renderer.GetAtlasBlendLightenPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kColorDodge:
      cmd.pipeline = renderer.GetAtlasBlendColorDodgePipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kColorBurn:
      cmd.pipeline = renderer.GetAtlasBlendColorBurnPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kHardLight:
      cmd.pipeline = renderer.GetAtlasBlendHardLightPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kSoftLight:
      cmd.pipeline = renderer.GetAtlasBlendSoftLightPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kDifference:
      cmd.pipeline = renderer.GetAtlasBlendDifferencePipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kExclusion:
      cmd.pipeline = renderer.GetAtlasBlendExclusionPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kMultiply:
      cmd.pipeline = renderer.GetAtlasBlendMultiplyPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kHue:
      cmd.pipeline = renderer.GetAtlasBlendHuePipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kSaturation:
      cmd.pipeline = renderer.GetAtlasBlendSaturationPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kColor:
      cmd.pipeline = renderer.GetAtlasBlendColorPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
    case BlendMode::kLuminosity:
      cmd.pipeline = renderer.GetAtlasBlendLuminosityPipeline(
          OptionsFromPassAndEntity(pass, entity));
      break;
  }
  // TODO: skip this step if we're given an image shader.
  auto src_texture = src_contents_->RenderToSnapshot(renderer, entity);
  if (!src_texture.has_value()) {
    return false;
  }

  VS::FrameInfo frame_info;
  frame_info.mvp = geometry_result.transform;

  FS::FragInfo frag_info;
  frag_info.src_y_coord_scale = src_texture->texture->GetYCoordScale();
  frag_info.alpha = alpha_;

  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(geometry_result.vertex_buffer);
  VS::BindFrameInfo(cmd, host_buffer.EmplaceUniform(frame_info));
  FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));
  FS::BindTextureSamplerSrc(
      cmd, src_texture.value().texture,
      renderer.GetContext()->GetSamplerLibrary()->GetSampler({}));

  return pass.AddCommand(std::move(cmd));
}

bool VerticesContents::RenderDestination(const ContentContext& renderer,
                                         const Entity& entity,
                                         RenderPass& pass) const {
  using VS = GeometryColorPipeline::VertexShader;
  using FS = GeometryColorPipeline::FragmentShader;

  auto& host_buffer = pass.GetTransientsBuffer();

  Command cmd;
  cmd.label = "Vertices";
  cmd.stencil_reference = entity.GetStencilDepth();

  auto opts = OptionsFromPassAndEntity(pass, entity);

  auto geometry_result =
      geometry_->GetPositionColorBuffer(renderer, entity, pass);
  opts.primitive_type = geometry_result.type;
  cmd.pipeline = renderer.GetGeometryColorPipeline(opts);
  cmd.BindVertices(geometry_result.vertex_buffer);

  VS::VertInfo vert_info;
  vert_info.mvp = geometry_result.transform;
  VS::BindVertInfo(cmd, host_buffer.EmplaceUniform(vert_info));

  FS::FragInfo frag_info;
  frag_info.alpha = alpha_;
  FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));

  return pass.AddCommand(std::move(cmd));
}

bool VerticesContents::RenderSource(const ContentContext& renderer,
                                    const Entity& entity,
                                    RenderPass& pass) const {
  using VS = TextureFillVertexShader;
  using FS = TextureFillFragmentShader;

  // TODO: skip this step if we're given an image shader.
  auto src_texture = src_contents_->RenderToSnapshot(renderer, entity);
  if (!src_texture.has_value()) {
    return false;
  }

  auto& host_buffer = pass.GetTransientsBuffer();

  Command cmd;
  cmd.label = "Vertices";
  cmd.stencil_reference = entity.GetStencilDepth();

  auto opts = OptionsFromPassAndEntity(pass, entity);

  auto geometry_result = geometry_->GetPositionUVBuffer(renderer, entity, pass);
  opts.primitive_type = geometry_result.type;
  cmd.pipeline = renderer.GetTexturePipeline(opts);
  cmd.BindVertices(geometry_result.vertex_buffer);

  VS::VertInfo vert_info;
  vert_info.mvp = geometry_result.transform;

  FS::FragInfo frag_info;
  frag_info.texture_sampler_y_coord_scale =
      src_texture->texture->GetYCoordScale();
  frag_info.alpha = alpha_;

  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(geometry_result.vertex_buffer);
  VS::BindVertInfo(cmd, host_buffer.EmplaceUniform(vert_info));
  FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));
  FS::BindTextureSampler(
      cmd, src_texture.value().texture,
      renderer.GetContext()->GetSamplerLibrary()->GetSampler({}));

  return pass.AddCommand(std::move(cmd));
}

}  // namespace impeller
