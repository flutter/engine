// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <optional>
#include <utility>

#include "impeller/entity/contents/filters/color_filter_contents.h"
#include "impeller/entity/contents/filters/filter_contents.h"
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
  return ComputeBoundingBox().TransformBounds(entity.GetTransformation());
}

Rect AtlasContents::ComputeBoundingBox() const {
  Rect bounding_box = {};
  for (size_t i = 0; i < texture_coords_.size(); i++) {
    auto matrix = transforms_[i];
    auto sample_rect = texture_coords_[i];
    auto bounds = Rect::MakeSize(sample_rect.size).TransformBounds(matrix);
    bounding_box = bounds.Union(bounding_box);
  }
  return bounding_box;
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
    return RenderTexture(renderer, entity, pass);
  }
  if (blend_mode_ == BlendMode::kDestination) {
    return RenderColors(renderer, entity, pass);
  }

  // Simple blends.
  if (blend_mode_ < BlendMode::kScreen) {
    if (!RenderColors(renderer, entity, pass)) {
      return false;
    }
    return RenderTexture(renderer, entity, pass, true);
  }

  // Ensure that we use the actual computed bounds and not a cull-rect
  // approximation of them.
  auto coverage = ComputeBoundingBox();
  auto size = coverage.size;

  auto dst_texture = renderer.MakeSubpass(
      ISize::Ceil(size),
      [&contents = *this, &coverage](const ContentContext& renderer,
                                     RenderPass& pass) -> bool {
        Entity sub_entity;
        sub_entity.SetBlendMode(BlendMode::kSourceOver);
        sub_entity.SetTransformation(
            Matrix::MakeTranslation(Vector3(-coverage.origin)));
        return contents.RenderColors(renderer, sub_entity, pass);
      });
  auto src_texture = renderer.MakeSubpass(
      ISize::Ceil(size),
      [&contents = *this, &coverage](const ContentContext& renderer,
                                     RenderPass& pass) -> bool {
        Entity sub_entity;
        sub_entity.SetBlendMode(BlendMode::kSourceOver);
        sub_entity.SetTransformation(
            Matrix::MakeTranslation(Vector3(-coverage.origin)));
        return contents.RenderTexture(renderer, sub_entity, pass);
      });

  if (!src_texture || !dst_texture) {
    return false;
  }
  auto contents = ColorFilterContents::MakeBlend(
      blend_mode_,
      {FilterInput::Make(src_texture), FilterInput::Make(dst_texture)});
  return contents->Render(renderer, entity, pass);
}

bool AtlasContents::RenderColors(const ContentContext& renderer,
                                 const Entity& entity,
                                 RenderPass& pass) const {
  using VS = GeometryColorPipeline::VertexShader;
  using FS = GeometryColorPipeline::FragmentShader;

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

  FS::FragInfo frag_info;
  frag_info.alpha = 1.0;

  auto opts = OptionsFromPassAndEntity(pass, entity);
  opts.blend_mode = BlendMode::kSourceOver;
  cmd.pipeline = renderer.GetGeometryColorPipeline(opts);
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(vertex_builder.CreateVertexBuffer(host_buffer));
  VS::BindVertInfo(cmd, host_buffer.EmplaceUniform(vert_info));
  FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));
  return pass.AddCommand(std::move(cmd));
}

bool AtlasContents::RenderTexture(const ContentContext& renderer,
                                  const Entity& entity,
                                  RenderPass& pass,
                                  bool apply_blend) const {
  using VS = TextureFillVertexShader;
  using FS = TextureFillFragmentShader;

  const auto texture_size = texture_->GetSize();
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

  auto options = OptionsFromPassAndEntity(pass, entity);
  if (apply_blend) {
    options.blend_mode = blend_mode_;
  }
  cmd.pipeline = renderer.GetTexturePipeline(options);
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
