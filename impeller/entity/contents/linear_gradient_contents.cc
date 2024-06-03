// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "linear_gradient_contents.h"

#include "impeller/core/formats.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/gradient_generator.h"
#include "impeller/entity/entity.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/vertex_buffer_builder.h"

namespace impeller {

LinearGradientContents::LinearGradientContents() = default;

LinearGradientContents::~LinearGradientContents() = default;

void LinearGradientContents::SetEndPoints(Point start_point, Point end_point) {
  start_point_ = start_point;
  end_point_ = end_point;
}

void LinearGradientContents::SetColors(std::vector<Color> colors) {
  colors_ = std::move(colors);
}

void LinearGradientContents::SetStops(std::vector<Scalar> stops) {
  stops_ = std::move(stops);
}

const std::vector<Color>& LinearGradientContents::GetColors() const {
  return colors_;
}

const std::vector<Scalar>& LinearGradientContents::GetStops() const {
  return stops_;
}

void LinearGradientContents::SetTileMode(Entity::TileMode tile_mode) {
  tile_mode_ = tile_mode;
}

bool LinearGradientContents::IsOpaque() const {
  if (GetOpacityFactor() < 1 || tile_mode_ == Entity::TileMode::kDecal) {
    return false;
  }
  for (auto color : colors_) {
    if (!color.IsOpaque()) {
      return false;
    }
  }
  return true;
}

// A much faster (in terms of ALU) linear gradient that uses vertex
// interpolation to perform all color computation. Requires that the geometry of
// the gradient is divided into regions based on the stop values.
// Currently restricted to rect geometry where the start and end points are
// perfectly horizontal/vertical, but could easily be expanded to StC cases
// provided that the start/end are on our outside the coverage rect.
bool LinearGradientContents::FastLinearGradient(const ContentContext& renderer,
                                                const Entity& entity,
                                                RenderPass& pass) const {
  using VS = GradientPipeline::VertexShader;
  using FS = GradientPipeline::FragmentShader;

  auto options = OptionsFromPassAndEntity(pass, entity);
  options.primitive_type = PrimitiveType::kTriangle;
  Geometry& geometry = *GetGeometry();

  // We already know this is an axis aligned rectangle, so the coverage will
  // be approximately the same as the geometry. For non AARs, we can force
  // stencil then cover (not done here). We give an identity transform to
  // avoid double transforming the gradient.
  std::optional<Rect> maybe_rect = geometry.GetCoverage(Matrix());
  if (!maybe_rect.has_value()) {
    return false;
  }
  Rect rect = maybe_rect.value();
  bool horizontal_axis = start_point_.y == end_point_.y;

  // Compute the locations of each breakpoint along the primary axis, then
  // create a rectangle that joins each segment. That will be two triangles
  // between each pair of points.
  VertexBufferBuilder<VS::PerVertexData> vtx_builder;
  vtx_builder.Reserve(6 * (stops_.size() - 1));
  Point prev = start_point_;
  for (auto i = 1u; i < stops_.size(); i++) {
    Scalar t = stops_[i];
    Point current = (1.0 - t) * start_point_ + t * end_point_;
    Rect section = horizontal_axis
                       ? Rect::MakeXYWH(prev.x, rect.GetY(), current.x - prev.x,
                                        rect.GetHeight())

                       : Rect::MakeXYWH(rect.GetX(), prev.y, rect.GetWidth(),
                                        current.y - prev.y);
    vtx_builder.AddVertices({
        {section.GetLeftTop(), colors_[i - 1]},
        {section.GetRightTop(), horizontal_axis ? colors_[i] : colors_[i - 1]},
        {section.GetLeftBottom(),
         horizontal_axis ? colors_[i - 1] : colors_[i]},
        {section.GetRightTop(), horizontal_axis ? colors_[i] : colors_[i - 1]},
        {section.GetLeftBottom(),
         horizontal_axis ? colors_[i - 1] : colors_[i]},
        {section.GetRightBottom(), colors_[i]},
    });
    prev = current;
  }
  auto& host_buffer = renderer.GetTransientsBuffer();

  pass.SetLabel("LinearGradient");
  pass.SetVertexBuffer(vtx_builder.CreateVertexBuffer(host_buffer));
  pass.SetPipeline(renderer.GetGradientPipeline(options));
  pass.SetStencilReference(0);

  // Take the pre-populated vertex shader uniform struct and set managed
  // values.
  VS::FrameInfo frame_info;
  frame_info.mvp = entity.GetShaderTransform(pass);

  VS::BindFrameInfo(pass, host_buffer.EmplaceUniform(frame_info));

  FS::FragInfo frag_info;
  frag_info.alpha =
      GetOpacityFactor() * GetGeometry()->ComputeAlphaCoverage(entity);

  FS::BindFragInfo(pass, host_buffer.EmplaceUniform(frag_info));

  return pass.Draw().ok();
}

bool LinearGradientContents::Render(const ContentContext& renderer,
                                    const Entity& entity,
                                    RenderPass& pass) const {
  if (GetGeometry()->IsAxisAlignedRect() &&
      (start_point_.x == end_point_.x || start_point_.y == end_point_.y) &&
      GetInverseEffectTransform().IsIdentity()) {
    return FastLinearGradient(renderer, entity, pass);
  }
  if (renderer.GetDeviceCapabilities().SupportsSSBO()) {
    return RenderSSBO(renderer, entity, pass);
  }
  return RenderTexture(renderer, entity, pass);
}

bool LinearGradientContents::RenderTexture(const ContentContext& renderer,
                                           const Entity& entity,
                                           RenderPass& pass) const {
  using VS = LinearGradientFillPipeline::VertexShader;
  using FS = LinearGradientFillPipeline::FragmentShader;

  VS::FrameInfo frame_info;
  frame_info.matrix = GetInverseEffectTransform();

  PipelineBuilderCallback pipeline_callback =
      [&renderer](ContentContextOptions options) {
        return renderer.GetLinearGradientFillPipeline(options);
      };
  return ColorSourceContents::DrawGeometry<VS>(
      renderer, entity, pass, pipeline_callback, frame_info,
      [this, &renderer, &entity](RenderPass& pass) {
        auto gradient_data = CreateGradientBuffer(colors_, stops_);
        auto gradient_texture =
            CreateGradientTexture(gradient_data, renderer.GetContext());
        if (gradient_texture == nullptr) {
          return false;
        }

        FS::FragInfo frag_info;
        frag_info.start_point = start_point_;
        frag_info.end_point = end_point_;
        frag_info.tile_mode = static_cast<Scalar>(tile_mode_);
        frag_info.decal_border_color = decal_border_color_;
        frag_info.texture_sampler_y_coord_scale =
            gradient_texture->GetYCoordScale();
        frag_info.alpha =
            GetOpacityFactor() * GetGeometry()->ComputeAlphaCoverage(entity);
        ;
        frag_info.half_texel =
            Vector2(0.5 / gradient_texture->GetSize().width,
                    0.5 / gradient_texture->GetSize().height);

        pass.SetCommandLabel("LinearGradientFill");

        SamplerDescriptor sampler_desc;
        sampler_desc.min_filter = MinMagFilter::kLinear;
        sampler_desc.mag_filter = MinMagFilter::kLinear;

        FS::BindTextureSampler(
            pass, std::move(gradient_texture),
            renderer.GetContext()->GetSamplerLibrary()->GetSampler(
                sampler_desc));
        FS::BindFragInfo(
            pass, renderer.GetTransientsBuffer().EmplaceUniform(frag_info));
        return true;
      });
}

namespace {
Scalar CalculateInverseDotStartToEnd(Point start_point, Point end_point) {
  Point start_to_end = end_point - start_point;
  Scalar dot =
      (start_to_end.x * start_to_end.x + start_to_end.y * start_to_end.y);
  return dot == 0.0f ? 0.0f : 1.0f / dot;
}
}  // namespace

bool LinearGradientContents::RenderSSBO(const ContentContext& renderer,
                                        const Entity& entity,
                                        RenderPass& pass) const {
  using VS = LinearGradientSSBOFillPipeline::VertexShader;
  using FS = LinearGradientSSBOFillPipeline::FragmentShader;

  VS::FrameInfo frame_info;
  frame_info.matrix = GetInverseEffectTransform();

  PipelineBuilderCallback pipeline_callback =
      [&renderer](ContentContextOptions options) {
        return renderer.GetLinearGradientSSBOFillPipeline(options);
      };
  return ColorSourceContents::DrawGeometry<VS>(
      renderer, entity, pass, pipeline_callback, frame_info,
      [this, &renderer, &entity](RenderPass& pass) {
        FS::FragInfo frag_info;
        frag_info.start_point = start_point_;
        frag_info.end_point = end_point_;
        frag_info.tile_mode = static_cast<Scalar>(tile_mode_);
        frag_info.decal_border_color = decal_border_color_;
        frag_info.alpha =
            GetOpacityFactor() * GetGeometry()->ComputeAlphaCoverage(entity);
        frag_info.start_to_end = end_point_ - start_point_;
        frag_info.inverse_dot_start_to_end =
            CalculateInverseDotStartToEnd(start_point_, end_point_);

        auto& host_buffer = renderer.GetTransientsBuffer();
        auto colors = CreateGradientColors(colors_, stops_);

        frag_info.colors_length = colors.size();
        auto color_buffer =
            host_buffer.Emplace(colors.data(), colors.size() * sizeof(StopData),
                                DefaultUniformAlignment());

        pass.SetCommandLabel("LinearGradientSSBOFill");

        FS::BindFragInfo(
            pass, renderer.GetTransientsBuffer().EmplaceUniform(frag_info));
        FS::BindColorData(pass, color_buffer);

        return true;
      });
}

bool LinearGradientContents::ApplyColorFilter(
    const ColorFilterProc& color_filter_proc) {
  for (Color& color : colors_) {
    color = color_filter_proc(color);
  }
  decal_border_color_ = color_filter_proc(decal_border_color_);
  return true;
}

}  // namespace impeller
