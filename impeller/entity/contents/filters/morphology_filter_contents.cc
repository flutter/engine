// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/morphology_filter_contents.h"

#include <cmath>

#include "impeller/entity/contents/anonymous_contents.h"
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
  direction_ = direction.Normalize();
  if (direction_.IsZero()) {
    direction_ = Vector2(0, 1);
  }
}

void DirectionalMorphologyFilterContents::SetMorphType(MorphType morph_type) {
  morph_type_ = morph_type;
}

std::optional<Entity> DirectionalMorphologyFilterContents::RenderFilter(
    const FilterInput::Vector& inputs,
    const ContentContext& renderer,
    const Entity& entity,
    const Matrix& effect_transform,
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
    return Entity::FromSnapshot(input_snapshot.value(), entity.GetBlendMode(),
                                entity.GetStencilDepth());
  }

  //----------------------------------------------------------------------------
  /// Create AnonymousContents for rendering.
  ///
  RenderProc render_proc = [input_snapshot, coverage, effect_transform,
                            morph_type = morph_type_, direction = direction_,
                            radius = radius_](const ContentContext& renderer,
                                              const Entity& entity,
                                              RenderPass& pass) -> bool {
    Command cmd;
    cmd.label = "Morphology Filter";
    cmd.stencil_reference = entity.GetStencilDepth();

    auto options = OptionsFromPass(pass);
    options.blend_mode = BlendMode::kSource;
    cmd.pipeline = renderer.GetMorphologyFilterPipeline(options);

    VertexBufferBuilder<VS::PerVertexData> vtx_builder;
    vtx_builder.AddVertices({
        {coverage.origin, Point(0, 0)},
        {{coverage.origin.x + coverage.size.width, coverage.origin.y},
         Point(1, 0)},
        {{coverage.origin.x + coverage.size.width,
          coverage.origin.y + coverage.size.height},
         Point(1, 1)},
        {coverage.origin, Point(0, 0)},
        {{coverage.origin.x + coverage.size.width,
          coverage.origin.y + coverage.size.height},
         Point(1, 1)},
        {{coverage.origin.x, coverage.origin.y + coverage.size.height},
         Point(0, 1)},
    });

    auto& host_buffer = pass.GetTransientsBuffer();
    auto vtx_buffer = vtx_builder.CreateVertexBuffer(host_buffer);

    VS::FrameInfo frame_info;
    frame_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize());
    frame_info.texture_sampler_y_coord_scale =
        input_snapshot->texture->GetYCoordScale();

    auto transform = entity.GetTransformation() * effect_transform.Basis();
    auto transformed_radius =
        transform.TransformDirection(direction * radius.radius);
    auto transformed_texture_vertices =
        Rect(Size(input_snapshot->texture->GetSize()))
            .GetTransformedPoints(input_snapshot->transform);
    auto transformed_texture_width =
        transformed_texture_vertices[0].GetDistance(
            transformed_texture_vertices[1]);
    auto transformed_texture_height =
        transformed_texture_vertices[0].GetDistance(
            transformed_texture_vertices[2]);

    FS::FragInfo frag_info;
    frag_info.radius = std::round(transformed_radius.GetLength());
    frag_info.morph_type = static_cast<Scalar>(morph_type);
    frag_info.uv_offset =
        input_snapshot->transform.Invert()
            .TransformDirection(transformed_radius)
            .Normalize() /
        Point(transformed_texture_width, transformed_texture_height);

    cmd.BindVertices(vtx_buffer);

    auto sampler_descriptor = input_snapshot->sampler_descriptor;
    if (renderer.GetDeviceCapabilities().SupportsDecalTileMode()) {
      sampler_descriptor.width_address_mode = SamplerAddressMode::kDecal;
      sampler_descriptor.height_address_mode = SamplerAddressMode::kDecal;
    }

    FS::BindTextureSampler(
        cmd, input_snapshot->texture,
        renderer.GetContext()->GetSamplerLibrary()->GetSampler(
            sampler_descriptor));
    VS::BindFrameInfo(cmd, host_buffer.EmplaceUniform(frame_info));
    FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));

    return pass.AddCommand(cmd);
  };

  CoverageProc coverage_proc =
      [coverage](const Entity& entity) -> std::optional<Rect> {
    return coverage;
  };

  auto contents = AnonymousContents::Make(render_proc, coverage_proc);

  Entity sub_entity;
  sub_entity.SetContents(std::move(contents));
  sub_entity.SetStencilDepth(entity.GetStencilDepth());
  sub_entity.SetTransformation(Matrix::MakeTranslation(coverage.origin) *
                               entity.GetTransformation());
  sub_entity.SetBlendMode(entity.GetBlendMode());
  return sub_entity;
}

std::optional<Rect> DirectionalMorphologyFilterContents::GetFilterCoverage(
    const FilterInput::Vector& inputs,
    const Entity& entity,
    const Matrix& effect_transform) const {
  if (inputs.empty()) {
    return std::nullopt;
  }

  auto coverage = inputs[0]->GetCoverage(entity);
  if (!coverage.has_value()) {
    return std::nullopt;
  }
  auto transform = inputs[0]->GetTransform(entity) * effect_transform.Basis();
  auto transformed_vector =
      transform.TransformDirection(direction_ * radius_.radius).Abs();

  auto origin = coverage->origin;
  auto size = Vector2(coverage->size);
  switch (morph_type_) {
    case FilterContents::MorphType::kDilate:
      origin -= transformed_vector;
      size += transformed_vector * 2;
      break;
    case FilterContents::MorphType::kErode:
      origin += transformed_vector;
      size -= transformed_vector * 2;
      break;
  }
  if (size.x < 0 || size.y < 0) {
    return Rect::MakeSize(Size(0, 0));
  }
  return Rect(origin, Size(size.x, size.y));
}

}  // namespace impeller
