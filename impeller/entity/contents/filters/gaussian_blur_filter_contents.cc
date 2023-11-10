// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/gaussian_blur_filter_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/texture_fill.frag.h"
#include "impeller/entity/texture_fill.vert.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"

namespace impeller {

using GaussianBlurVertexShader = GaussianBlurPipeline::VertexShader;
using GaussianBlurFragmentShader = GaussianBlurPipeline::FragmentShader;

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
/// @param effect_transform The transform passed into |RenderFilter|. This is
/// applied to the rect for the whole texture before being transformed into the
/// uv space.
/// @param entity_transform The local transform of the entity.
/// @return A quad in uv coordinates representing the whole |subpass_size|.
std::array<Point, 4> CalculateUVs(const ISize& subpass_size,
                                  const Matrix& effect_transform,
                                  const Matrix& entity_transform) {
  Rect pass_rect =
      Rect::MakeSize(subpass_size).TransformBounds(effect_transform);
  Matrix transform = Matrix::MakeScale({1.0f / pass_rect.size.width,
                                        1.0f / pass_rect.size.height, 1.0f}) *
                     entity_transform.Invert();
  return pass_rect.GetTransformedPoints(transform);
}

template <typename T>
void BindVertices(Command& cmd,
                  HostBuffer& host_buffer,
                  std::initializer_list<typename T::PerVertexData>&& vertices) {
  VertexBufferBuilder<typename T::PerVertexData> vtx_builder;
  vtx_builder.AddVertices(vertices);
  auto vtx_buffer = vtx_builder.CreateVertexBuffer(host_buffer);
  cmd.BindVertices(vtx_buffer);
}

std::shared_ptr<Texture> MakeDownsampleSubpass(
    const ContentContext& renderer,
    std::shared_ptr<Texture> input_texture,
    const SamplerDescriptor& sampler_descriptor,
    const Matrix& effect_transform,
    const Matrix& entity_transform,
    const Vector2& downsample) {
  ISize subpass_size = ISize(input_texture->GetSize().width / downsample.x,
                             input_texture->GetSize().height / downsample.y);
  ContentContext::SubpassCallback subpass_callback =
      [&](const ContentContext& renderer, RenderPass& pass) {
        HostBuffer& host_buffer = pass.GetTransientsBuffer();

        std::array<Point, 4> uvs =
            CalculateUVs(subpass_size,
                         Matrix::MakeScale({downsample.x, downsample.y, 1.0}) *
                             effect_transform,
                         entity_transform);

        Command cmd;
        DEBUG_COMMAND_INFO(cmd, "Gaussian blur downsample");
        auto pipeline_options = OptionsFromPass(pass);
        pipeline_options.primitive_type = PrimitiveType::kTriangleStrip;
        cmd.pipeline = renderer.GetTexturePipeline(pipeline_options);

        TextureFillVertexShader::FrameInfo frame_info;
        frame_info.mvp = Matrix::MakeOrthographic(ISize(1, 1));
        frame_info.texture_sampler_y_coord_scale = 1.0;
        frame_info.alpha = 1.0;

        BindVertices<TextureFillVertexShader>(cmd, host_buffer,
                                              {
                                                  {Point(0, 0), uvs[0]},
                                                  {Point(1, 0), uvs[1]},
                                                  {Point(0, 1), uvs[2]},
                                                  {Point(1, 1), uvs[3]},
                                              });

        TextureFillVertexShader::BindFrameInfo(
            cmd, host_buffer.EmplaceUniform(frame_info));
        TextureFillFragmentShader::BindTextureSampler(
            cmd, input_texture,
            renderer.GetContext()->GetSamplerLibrary()->GetSampler(
                sampler_descriptor));

        pass.AddCommand(std::move(cmd));

        return true;
      };
  std::shared_ptr<Texture> out_texture = renderer.MakeSubpass(
      "Gaussian Blur Filter", subpass_size, subpass_callback);
  return out_texture;
}

std::shared_ptr<Texture> MakeBlurSubpass(
    const ContentContext& renderer,
    std::shared_ptr<Texture> input_texture,
    const SamplerDescriptor& sampler_descriptor,
    const Matrix& effect_transform,
    const Matrix& entity_transform,
    const GaussianBlurFragmentShader::BlurInfo& blur_info,
    const Vector2& downsample) {
  ISize subpass_size = ISize(input_texture->GetSize().width / downsample.x,
                             input_texture->GetSize().height / downsample.y);
  ContentContext::SubpassCallback subpass_callback =
      [&](const ContentContext& renderer, RenderPass& pass) {
        GaussianBlurVertexShader::FrameInfo frame_info{
            .mvp = Matrix::MakeOrthographic(ISize(1, 1)),
            .texture_sampler_y_coord_scale = 1.0};

        HostBuffer& host_buffer = pass.GetTransientsBuffer();

        std::array<Point, 4> uvs =
            CalculateUVs(subpass_size,
                         Matrix::MakeScale({downsample.x, downsample.y, 1.0}) *
                             effect_transform,
                         entity_transform);

        Command cmd;
        auto options = OptionsFromPass(pass);
        cmd.pipeline = renderer.GetGaussianBlurPipeline(options);
        BindVertices<GaussianBlurVertexShader>(cmd, host_buffer,
                                               {
                                                   {Point(0, 0), uvs[0]},
                                                   {Point(1, 0), uvs[1]},
                                                   {Point(0, 1), uvs[2]},
                                                   {Point(1, 1), uvs[3]},
                                               });

        GaussianBlurFragmentShader::BindTextureSampler(
            cmd, input_texture,
            renderer.GetContext()->GetSamplerLibrary()->GetSampler(
                sampler_descriptor));
        GaussianBlurVertexShader::BindFrameInfo(
            cmd, host_buffer.EmplaceUniform(frame_info));
        GaussianBlurFragmentShader::BindBlurInfo(
            cmd, host_buffer.EmplaceUniform(blur_info));
        pass.AddCommand(std::move(cmd));

        return true;
      };
  std::shared_ptr<Texture> out_texture = renderer.MakeSubpass(
      "Gaussian Blur Filter", subpass_size, subpass_callback);
  return out_texture;
}

/// Given a desired |scalar|, will return the scalar that gets close but leaves
/// |size| in integer sizes.
Vector2 CalculateIntegerScale(Scalar scalar, ISize size) {
  ISize new_size(size.width / scalar, size.height / scalar);
  return Vector2(size.width / static_cast<Scalar>(new_size.width),
                 size.height / static_cast<Scalar>(new_size.height));
}

/// Calculate how much to scale down the texture depending on the blur radius.
/// This curve was taken from |DirectionalGaussianBlurFilterContents|.
Scalar CalculateScale(Scalar radius) {
  constexpr Scalar decay = 4.0;   // Larger is more gradual.
  constexpr Scalar limit = 0.95;  // The maximum percentage of the scaledown.
  const Scalar curve =
      std::min(1.0, decay / (std::max(1.0f, radius) + decay - 1.0));
  return (curve - 1) * limit + 1;
};

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
  if (inputs.empty()) {
    return std::nullopt;
  }

  std::optional<Snapshot> input_snapshot =
      inputs[0]->GetSnapshot("GaussianBlur", renderer, entity,
                             /*coverage_hint=*/{});
  if (!input_snapshot.has_value()) {
    return std::nullopt;
  }

  Scalar blur_radius = CalculateBlurRadius(sigma_);
  Scalar desired_scale = 1.0 / CalculateScale(blur_radius);
  Vector2 downsample =
       CalculateIntegerScale(desired_scale, input_snapshot->texture->GetSize());

  std::shared_ptr<Texture> pass1_out_texture = MakeDownsampleSubpass(
      renderer, input_snapshot->texture, input_snapshot->sampler_descriptor,
      effect_transform, entity.GetTransformation(), downsample);

  Size pass1_pixel_size(1.0 / pass1_out_texture->GetSize().width,
                        1.0 / pass1_out_texture->GetSize().height);

  std::shared_ptr<Texture> pass2_out_texture = MakeBlurSubpass(
      renderer, pass1_out_texture, input_snapshot->sampler_descriptor,
      /*effect_transform=*/Matrix(),
      /*entity_transform=*/Matrix(),
      GaussianBlurFragmentShader::BlurInfo{
          .blur_uv_offset = Point(0.0, pass1_pixel_size.height),
          .blur_sigma = sigma_ / downsample.y,
          .blur_radius = blur_radius / downsample.y,
      },
      /*downsample=*/Vector2(1.0, 1.0));

  // TODO(gaaclarke): Make this pass reuse the texture from pass1.
  std::shared_ptr<Texture> pass3_out_texture = MakeBlurSubpass(
      renderer, pass2_out_texture, input_snapshot->sampler_descriptor,
      /*effect_transform=*/Matrix(), /*entity_transform=*/Matrix(),
      GaussianBlurFragmentShader::BlurInfo{
          .blur_uv_offset = Point(pass1_pixel_size.width, 0.0),
          .blur_sigma = sigma_ / downsample.x,
          .blur_radius = blur_radius / downsample.x,
      },
      /*downsample=*/Vector2(1.0, 1.0));

  SamplerDescriptor sampler_desc = MakeSamplerDescriptor(
      MinMagFilter::kLinear, SamplerAddressMode::kClampToEdge);

  return Entity::FromSnapshot(Snapshot{.texture = pass3_out_texture,
                                       .transform = Matrix::MakeScale(
                                           {downsample.x, downsample.y, 1.0}),
                                       .sampler_descriptor = sampler_desc,
                                       .opacity = input_snapshot->opacity},
                              entity.GetBlendMode(), entity.GetClipDepth());
}

Scalar GaussianBlurFilterContents::CalculateBlurRadius(Scalar sigma) {
  return static_cast<Radius>(Sigma(sigma)).radius;
}

}  // namespace impeller
