// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/gaussian_blur_filter_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"

namespace impeller {

namespace {
SamplerDescriptor MakeSamplerDescriptor(MinMagFilter filter,
                                        SamplerAddressMode address_mode) {
  SamplerDescriptor sampler_desc;
  sampler_desc.min_filter = filter;
  sampler_desc.mag_filter = filter;
  sampler_desc.width_address_mode = address_mode;
  sampler_desc.width_address_mode = address_mode;
  return sampler_desc;
}

/// @brief This performs the conversion from an entity's local coordinates to
///        a subpasses uv coordinates.
/// @param subpass_size The size of the subpass in pixels.
/// @param entity_transform The local transform of the entity.
/// @return A quad in uv coordinates.
std::array<Point, 4> CalculateUVs(const ISize& subpass_size,
                                  const Matrix& entity_transform) {
  Point offset = Matrix::MakeScale({1.0f / subpass_size.width,
                                    1.0f / subpass_size.height, 1.0f}) *
                 (entity_transform.Invert() * Point(0.0, 0.0));
  std::array<Point, 4> input_uvs = {Point(0, 0), Point(1, 0), Point(0, 1),
                                    Point(1, 1)};
  return {input_uvs[0] + offset, input_uvs[1] + offset, input_uvs[2] + offset,
          input_uvs[3] + offset};
}

}  // namespace

GaussianBlurFilterContents::GaussianBlurFilterContents(Scalar sigma)
    : sigma_(sigma) {}

std::optional<Rect> GaussianBlurFilterContents::GetFilterSourceCoverage(
    const Matrix& effect_transform,
    const Rect& output_limit) const {
  Scalar blur_radius = CalculateBlurRadius(sigma_);
  Vector3 blur_radii =
      effect_transform.Basis() * Vector3{blur_radius, blur_radius, 0.0};
  return output_limit.Expand(Point(blur_radii.x, blur_radii.y));
}

std::optional<Rect> GaussianBlurFilterContents::GetFilterCoverage(
    const FilterInput::Vector& inputs,
    const Entity& entity,
    const Matrix& effect_transform) const {
  if (inputs.empty()) {
    return {};
  }

  std::optional<Rect> input_coverage = inputs[0]->GetCoverage(entity);
  if (!input_coverage.has_value()) {
    return {};
  }

  Scalar blur_radius = CalculateBlurRadius(sigma_);
  Vector3 blur_radii =
      effect_transform.Basis() * Vector3{blur_radius, blur_radius, 0.0};
  return input_coverage.value().Expand(Point(blur_radii.x, blur_radii.y));
}

std::optional<Entity> GaussianBlurFilterContents::RenderFilter(
    const FilterInput::Vector& inputs,
    const ContentContext& renderer,
    const Entity& entity,
    const Matrix& effect_transform,
    const Rect& coverage,
    const std::optional<Rect>& coverage_hint) const {
  using GaussianBlurVertexShader = GaussianBlurPipeline::VertexShader;
  using GaussianBlurFragmentShader = GaussianBlurPipeline::FragmentShader;

  if (inputs.empty()) {
    return std::nullopt;
  }

  std::optional<Snapshot> input_snapshot =
      inputs[0]->GetSnapshot("GaussianBlur", renderer, entity,
                             /*coverage_hint=*/{});
  if (!input_snapshot.has_value()) {
    return std::nullopt;
  }

  ISize subpass_size = input_snapshot->texture->GetSize();
  ContentContext::SubpassCallback subpass_callback = [&](const ContentContext&
                                                             renderer,
                                                         RenderPass& pass) {
    GaussianBlurVertexShader::FrameInfo frame_info{
        .mvp = Matrix::MakeOrthographic(ISize(1, 1)),
        .texture_sampler_y_coord_scale = 1.0};

    GaussianBlurFragmentShader::BlurInfo blur_info{
        .blur_uv_offset = Point(),
        .blur_sigma = 0.0,
        .blur_radius = 1.0,
    };

    HostBuffer& host_buffer = pass.GetTransientsBuffer();

    std::array<Point, 4> uvs =
        CalculateUVs(subpass_size, entity.GetTransformation());
    VertexBufferBuilder<GaussianBlurVertexShader::PerVertexData> vtx_builder;
    vtx_builder.AddVertices({
        {Point(0, 0), uvs[0]},
        {Point(1, 0), uvs[1]},
        {Point(0, 1), uvs[2]},
        {Point(1, 1), uvs[3]},
    });
    auto vtx_buffer = vtx_builder.CreateVertexBuffer(host_buffer);

    Command cmd;
    auto options = OptionsFromPass(pass);
    cmd.pipeline = renderer.GetGaussianBlurPipeline(options);
    cmd.BindVertices(vtx_buffer);

    GaussianBlurFragmentShader::BindTextureSampler(
        cmd, input_snapshot->texture,
        renderer.GetContext()->GetSamplerLibrary()->GetSampler(
            input_snapshot->sampler_descriptor));
    GaussianBlurVertexShader::BindFrameInfo(
        cmd, host_buffer.EmplaceUniform(frame_info));
    GaussianBlurFragmentShader::BindBlurInfo(
        cmd, host_buffer.EmplaceUniform(blur_info));
    pass.AddCommand(std::move(cmd));

    return true;
  };
  auto out_texture = renderer.MakeSubpass("Directional Gaussian Blur Filter",
                                          subpass_size, subpass_callback);

  SamplerDescriptor sampler_desc = MakeSamplerDescriptor(
      MinMagFilter::kLinear, SamplerAddressMode::kClampToEdge);

  return Entity::FromSnapshot(Snapshot{.texture = out_texture,
                                       .transform = Matrix(),
                                       .sampler_descriptor = sampler_desc,
                                       .opacity = input_snapshot->opacity},
                              entity.GetBlendMode(), entity.GetClipDepth());
}

Scalar GaussianBlurFilterContents::CalculateBlurRadius(Scalar sigma) {
  return static_cast<Radius>(Sigma(sigma)).radius;
}

}  // namespace impeller
