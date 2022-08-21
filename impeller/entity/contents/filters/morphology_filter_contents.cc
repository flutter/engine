// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/morphology_filter_contents.h"
#include <cmath>
#include "impeller/entity/contents/content_context.h"

#include "impeller/entity/contents/contents.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"

namespace impeller {

DirectionalMorphologyFilterContents::DirectionalMorphologyFilterContents() =
    default;

DirectionalMorphologyFilterContents::~DirectionalMorphologyFilterContents() =
    default;

void DirectionalMorphologyFilterContents::SetRadius(Radius radius) {
  radius_ = radius;
}

void DirectionalMorphologyFilterContents::SetDirection(Vector2 direction) {
  direction_ = direction;
}

void DirectionalMorphologyFilterContents::SetMorphType(MorphType morph_type) {
  morph_type_ = morph_type;
}

std::optional<Snapshot> DirectionalMorphologyFilterContents::RenderFilter(
    const FilterInput::Vector& inputs,
    const ContentContext& renderer,
    const Entity& entity,
    const Rect& coverage) const {
  using VS = MorphologyFilterPipeline::VertexShader;
  using FS = MorphologyFilterPipeline::FragmentShader;

  //----------------------------------------------------------------------------
  /// Handle inputs.
  ///

  if (inputs.empty()) {
    return std::nullopt;
  }

  auto input_snapshot = inputs[0]->GetSnapshot(renderer, entity);
  if (!input_snapshot.has_value()) {
    return std::nullopt;
  }

  if (radius_.radius < kEhCloseEnough) {
    return input_snapshot.value();
  }

  auto transformed_radius = entity.GetTransformation().TransformDirection(
      direction_ * radius_.radius);

  auto transformed_radius_length = transformed_radius.GetLength();

  // A matrix that rotates the snapshot space such that the direction is
  // +X.
  auto texture_rotate =
      Matrix::MakeRotationZ(transformed_radius.Normalize().AngleTo({1, 0}));

  // Converts local pass space to screen space. This is just the snapshot space
  // rotated such that the direction is +X.
  auto pass_transform = texture_rotate * input_snapshot->transform;

  // The pass texture coverage, but rotated such that the filter is in the +X
  // direction, and expanded to include the radius. This is used for UV
  // projection and as a source for the pass size. Note that it doesn't matter
  // which direction the space is rotated in when grabbing the pass size.
  auto pass_texture_rect = Rect::MakeSize(input_snapshot->texture->GetSize())
                               .TransformBounds(pass_transform);

  // auto original_pass_texture_rect = pass_texture_rect;
  pass_texture_rect.origin.x -= transformed_radius_length;
  pass_texture_rect.size.width += transformed_radius_length * 2;

  // UV mapping.
  auto pass_uv_project = [&texture_rotate,
                          &pass_texture_rect](Snapshot& input) {
    auto uv_matrix = Matrix::MakeScale(1 / Vector2(input.texture->GetSize())) *
                     (texture_rotate * input.transform).Invert();
    return pass_texture_rect.GetTransformedPoints(uv_matrix);
  };
  auto input_uvs = pass_uv_project(input_snapshot.value());

  auto transformed_texture_vertices =
      Rect(Size(input_snapshot->texture->GetSize()))
          .GetTransformedPoints(pass_transform);
  auto transformed_texture_width = transformed_texture_vertices[0].GetDistance(
      transformed_texture_vertices[1]);
  auto transformed_texture_height = transformed_texture_vertices[0].GetDistance(
      transformed_texture_vertices[2]);

  //----------------------------------------------------------------------------
  /// Render to texture.
  ///

  ContentContext::SubpassCallback callback = [&](const ContentContext& renderer,
                                                 RenderPass& pass) {
    auto& host_buffer = pass.GetTransientsBuffer();

    VertexBufferBuilder<VS::PerVertexData> vtx_builder;
    vtx_builder.AddVertices({
        {Point(0, 0), input_uvs[0]},
        {Point(1, 0), input_uvs[1]},
        {Point(1, 1), input_uvs[3]},
        {Point(0, 0), input_uvs[0]},
        {Point(1, 1), input_uvs[3]},
        {Point(0, 1), input_uvs[2]},
    });

    auto vtx_buffer = vtx_builder.CreateVertexBuffer(host_buffer);

    VS::FrameInfo frame_info;
    frame_info.mvp = Matrix::MakeOrthographic(ISize(1, 1));

    FS::FragInfo frag_info;
    frag_info.texture_sampler_y_coord_scale =
        input_snapshot->texture->GetYCoordScale();
    frag_info.radius = std::round(transformed_radius_length);
    // The direction is in input UV space.
    frag_info.direction =
        pass_transform.Invert().TransformDirection(Vector2(1, 0)).Normalize();
    frag_info.texture_size =
        Point(transformed_texture_width, transformed_texture_height);
    frag_info.morph_type = static_cast<Scalar>(morph_type_);

    Command cmd;
    cmd.label = "Morphology Filter";
    auto options = OptionsFromPass(pass);
    options.blend_mode = Entity::BlendMode::kSource;
    cmd.pipeline = renderer.GetMorphologyFilterPipeline(options);
    cmd.BindVertices(vtx_buffer);

    FS::BindTextureSampler(
        cmd, input_snapshot->texture,
        renderer.GetContext()->GetSamplerLibrary()->GetSampler(
            input_snapshot->sampler_descriptor));
    VS::BindFrameInfo(cmd, host_buffer.EmplaceUniform(frame_info));
    FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));

    return pass.AddCommand(cmd);
  };

  auto out_texture = renderer.MakeSubpass(
      ISize(pass_texture_rect.size.width, pass_texture_rect.size.height),
      callback);
  if (!out_texture) {
    return std::nullopt;
  }
  out_texture->SetLabel("DirectionalMorphologyFilter Texture");

  SamplerDescriptor sampler_desc;
  sampler_desc.min_filter = MinMagFilter::kLinear;
  sampler_desc.mag_filter = MinMagFilter::kLinear;

  return Snapshot{
      .texture = out_texture,
      .transform = texture_rotate.Invert() *
                   Matrix::MakeTranslation(pass_texture_rect.origin),
      .sampler_descriptor = sampler_desc};
}

std::optional<Rect> DirectionalMorphologyFilterContents::GetFilterCoverage(
    const FilterInput::Vector& inputs,
    const Entity& entity) const {
  if (inputs.empty()) {
    return std::nullopt;
  }

  auto coverage = inputs[0]->GetCoverage(entity);
  if (!coverage.has_value()) {
    return std::nullopt;
  }

  auto transformed_vector = inputs[0]
                                ->GetTransform(entity)
                                .TransformDirection(direction_ * radius_.radius)
                                .Abs();

  auto extent = coverage->size + transformed_vector * 2;
  return Rect(coverage->origin - transformed_vector, Size(extent.x, extent.y));
}

}  // namespace impeller
