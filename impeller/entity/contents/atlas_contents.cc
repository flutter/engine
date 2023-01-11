// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <optional>
#include <utility>

#include "impeller/renderer/formats.h"
#include "impeller/renderer/sampler_library.h"
#include "impeller/renderer/vertex_buffer_builder.h"

#include "impeller/entity/contents/atlas_contents.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/texture_fill.frag.h"
#include "impeller/entity/texture_fill.vert.h"
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

void AtlasContents::SetTransforms(std::vector<Matrix> transforms) {
  transforms_ = std::move(transforms);
}

void AtlasContents::SetTextureCoordinates(std::vector<Rect> texture_coords) {
  texture_coords_ = std::move(texture_coords);
}

void AtlasContents::SetColors(std::vector<Color> colors) {
  colors_ = std::move(colors);
}

void AtlasContents::SetAlpha(Scalar alpha) {
  alpha_ = alpha;
}

void AtlasContents::SetBlendMode(BlendMode blend_mode) {
  blend_mode_ = blend_mode;
}

void AtlasContents::SetCullRect(std::optional<Rect> cull_rect) {
  cull_rect_ = cull_rect;
}

std::optional<Rect> AtlasContents::GetCoverage(const Entity& entity) const {
  if (cull_rect_.has_value()) {
    return cull_rect_.value().TransformBounds(entity.GetTransformation());
  }

  Rect bounding_box = {};
  for (size_t i = 0; i < texture_coords_.size(); i++) {
    auto matrix = transforms_[i];
    auto sample_rect = texture_coords_[i];
    auto bounds = Rect::MakeSize(sample_rect.size).TransformBounds(matrix);
    bounding_box = bounds.Union(bounding_box);
  }
  return bounding_box.TransformBounds(entity.GetTransformation());
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
  if (texture_ == nullptr || blend_mode_ == BlendMode::kClear) {
    return true;
  }

  if (blend_mode_ == BlendMode::kSource || colors_.size() == 0) {
    return RenderNoColor(renderer, entity, pass);
  }
  if (blend_mode_ == BlendMode::kDestination) {
    return RenderOnlyColor(renderer, entity, pass);
  }

  using VS = AtlasBlendSrcOverPipeline::VertexShader;
  using FS = AtlasBlendSrcOverPipeline::FragmentShader;

  const auto texture_size = texture_->GetSize();
  if (texture_size.IsEmpty()) {
    return true;
  }

  VertexBufferBuilder<VS::PerVertexData> vertex_builder;
  vertex_builder.Reserve(texture_coords_.size() * 6);
  constexpr size_t indices[6] = {0, 1, 2, 1, 2, 3};
  constexpr Scalar width[6] = {0, 1, 0, 1, 0, 1};
  constexpr Scalar height[6] = {0, 0, 1, 0, 1, 1};
  for (size_t i = 0; i < texture_coords_.size(); i++) {
    auto sample_rect = texture_coords_[i];
    auto matrix = transforms_[i];
    auto color = colors_[i];
    auto transformed_points =
        Rect::MakeSize(sample_rect.size).GetTransformedPoints(matrix);

    for (size_t j = 0; j < 6; j++) {
      VS::PerVertexData data;
      data.vertices = transformed_points[indices[j]];
      data.src_texture_coords =
          (sample_rect.origin + Point(sample_rect.size.width * width[j],
                                      sample_rect.size.height * height[j])) /
          texture_size;
      data.dst_color = color;
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

  FS::FragInfo frag_info;
  frag_info.src_y_coord_scale = texture_->GetYCoordScale();
  frag_info.alpha = alpha_;

  Command cmd;
  cmd.label = "DrawAtlas";
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
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(vertex_builder.CreateVertexBuffer(host_buffer));
  VS::BindFrameInfo(cmd, host_buffer.EmplaceUniform(frame_info));
  FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));
  FS::BindTextureSamplerSrc(
      cmd, texture_,
      renderer.GetContext()->GetSamplerLibrary()->GetSampler(
          sampler_descriptor_));
  return pass.AddCommand(std::move(cmd));
}

bool AtlasContents::RenderOnlyColor(const ContentContext& renderer,
                                    const Entity& entity,
                                    RenderPass& pass) const {
  using VS = GeometryColorPipeline::VertexShader;

  const auto texture_size = texture_->GetSize();
  if (texture_size.IsEmpty()) {
    return true;
  }

  VertexBufferBuilder<VS::PerVertexData> vertex_builder;
  vertex_builder.Reserve(texture_coords_.size() * 6);
  constexpr size_t indices[6] = {0, 1, 2, 1, 2, 3};
  for (size_t i = 0; i < texture_coords_.size(); i++) {
    auto sample_rect = texture_coords_[i];
    auto matrix = transforms_[i];
    auto transformed_points =
        Rect::MakeSize(sample_rect.size).GetTransformedPoints(matrix);

    for (size_t j = 0; j < 6; j++) {
      VS::PerVertexData data;
      data.position = transformed_points[indices[j]];
      data.color = colors_[i].Premultiply();
      vertex_builder.AppendVertex(data);
    }
  }

  if (!vertex_builder.HasVertices()) {
    return true;
  }

  Command cmd;
  cmd.label = "DrawAtlas";

  auto& host_buffer = pass.GetTransientsBuffer();

  VS::VertInfo vert_info;
  vert_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                  entity.GetTransformation();

  cmd.pipeline =
      renderer.GetGeometryColorPipeline(OptionsFromPassAndEntity(pass, entity));
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(vertex_builder.CreateVertexBuffer(host_buffer));
  VS::BindVertInfo(cmd, host_buffer.EmplaceUniform(vert_info));
  return pass.AddCommand(std::move(cmd));
}

bool AtlasContents::RenderNoColor(const ContentContext& renderer,
                                  const Entity& entity,
                                  RenderPass& pass) const {
  using VS = TextureFillVertexShader;
  using FS = TextureFillFragmentShader;

  const auto texture_size = texture_->GetSize();
  if (texture_size.IsEmpty()) {
    return true;
  }

  VertexBufferBuilder<VS::PerVertexData> vertex_builder;
  vertex_builder.Reserve(texture_coords_.size() * 6);
  constexpr size_t indices[6] = {0, 1, 2, 1, 2, 3};
  constexpr Scalar width[6] = {0, 1, 0, 1, 0, 1};
  constexpr Scalar height[6] = {0, 0, 1, 0, 1, 1};
  for (size_t i = 0; i < texture_coords_.size(); i++) {
    auto sample_rect = texture_coords_[i];
    auto matrix = transforms_[i];
    auto transformed_points =
        Rect::MakeSize(sample_rect.size).GetTransformedPoints(matrix);

    for (size_t j = 0; j < 6; j++) {
      VS::PerVertexData data;
      data.position = transformed_points[indices[j]];
      data.texture_coords =
          (sample_rect.origin + Point(sample_rect.size.width * width[j],
                                      sample_rect.size.height * height[j])) /
          texture_size;
      vertex_builder.AppendVertex(data);
    }
  }

  if (!vertex_builder.HasVertices()) {
    return true;
  }

  Command cmd;
  cmd.label = "DrawAtlas";

  auto& host_buffer = pass.GetTransientsBuffer();

  VS::VertInfo vert_info;
  vert_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                  entity.GetTransformation();

  FS::FragInfo frag_info;
  frag_info.texture_sampler_y_coord_scale = texture_->GetYCoordScale();
  frag_info.alpha = alpha_;

  cmd.pipeline =
      renderer.GetTexturePipeline(OptionsFromPassAndEntity(pass, entity));
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(vertex_builder.CreateVertexBuffer(host_buffer));
  VS::BindVertInfo(cmd, host_buffer.EmplaceUniform(vert_info));
  FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));
  FS::BindTextureSampler(cmd, texture_,
                         renderer.GetContext()->GetSamplerLibrary()->GetSampler(
                             sampler_descriptor_));
  return pass.AddCommand(std::move(cmd));
}

}  // namespace impeller
