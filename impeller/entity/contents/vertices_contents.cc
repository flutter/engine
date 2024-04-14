// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vertices_contents.h"

#include "fml/logging.h"
#include "impeller/core/formats.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/entity/contents/filters/blend_filter_contents.h"
#include "impeller/entity/geometry/geometry.h"
#include "impeller/entity/geometry/vertices_geometry.h"
#include "impeller/geometry/color.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

namespace {
static std::optional<SamplerAddressMode> TileModeToAddressMode(
    Entity::TileMode tile_mode,
    const Capabilities& capabilities) {
  switch (tile_mode) {
    case Entity::TileMode::kClamp:
      return SamplerAddressMode::kClampToEdge;
      break;
    case Entity::TileMode::kMirror:
      return SamplerAddressMode::kMirror;
      break;
    case Entity::TileMode::kRepeat:
      return SamplerAddressMode::kRepeat;
      break;
    case Entity::TileMode::kDecal:
      if (capabilities.SupportsDecalSamplerAddressMode()) {
        return SamplerAddressMode::kDecal;
      }
      return std::nullopt;
  }
}
}  // namespace

VerticesContents::VerticesContents() {}

VerticesContents::~VerticesContents() {}

void VerticesContents::SetGeometry(std::shared_ptr<VerticesGeometry> geometry) {
  geometry_ = std::move(geometry);
}

void VerticesContents::SetAlpha(Scalar alpha) {
  alpha_ = alpha;
}

void VerticesContents::SetBlendMode(BlendMode blend_mode) {
  blend_mode_ = blend_mode;
}

void VerticesContents::SetTexture(std::shared_ptr<Texture> texture) {
  texture_ = std::move(texture);
}

std::optional<Rect> VerticesContents::GetCoverage(const Entity& entity) const {
  return geometry_->GetCoverage(entity.GetTransform());
}

void VerticesContents::SetSamplerDescriptor(SamplerDescriptor descriptor) {
  descriptor_ = std::move(descriptor);
}

void VerticesContents::SetTileMode(Entity::TileMode tile_mode_x,
                                   Entity::TileMode tile_mode_y) {
  tile_mode_x_ = tile_mode_x;
  tile_mode_y_ = tile_mode_y;
}

void VerticesContents::SetEffectTransform(Matrix transform) {
  inverse_matrix_ = transform.Invert();
}

bool VerticesContents::Render(const ContentContext& renderer,
                              const Entity& entity,
                              RenderPass& pass) const {
  FML_DCHECK(geometry_->HasVertexColors());
  if (blend_mode_ == BlendMode::kClear) {
    return true;
  }
  if (blend_mode_ <= BlendMode::kModulate) {
    return RenderPorterDuff(renderer, entity, pass);
  }
  return RenderAdvanced(renderer, entity, pass);
}

bool VerticesContents::RenderPorterDuff(const ContentContext& renderer,
                                        const Entity& entity,
                                        RenderPass& pass) const {
  FML_DCHECK(geometry_->HasVertexColors());
  if (blend_mode_ == BlendMode::kClear) {
    return true;
  }

  using VS = PorterDuffBlendPipeline::VertexShader;
  using FS = PorterDuffBlendPipeline::FragmentShader;

  Rect texture_coverage = (!!texture_) ? Rect::MakeSize(texture_->GetSize())
                                       : Rect::MakeLTRB(0, 0, 1, 1);
  GeometryResult geometry_result = geometry_->GetPositionUVColorBuffer(
      texture_coverage, inverse_matrix_, renderer, entity, pass);
  if (geometry_result.vertex_buffer.vertex_count == 0) {
    return true;
  }
  FML_DCHECK(geometry_result.mode == GeometryResult::Mode::kNormal);

#ifdef IMPELLER_DEBUG
  pass.SetCommandLabel(SPrintF("DrawVertices Porterduff Blend (%s)",
                               BlendModeToString(blend_mode_)));
#endif  // IMPELLER_DEBUG
  pass.SetVertexBuffer(std::move(geometry_result.vertex_buffer));

  auto options = OptionsFromPassAndEntity(pass, entity);
  options.primitive_type = geometry_result.type;
  pass.SetPipeline(renderer.GetPorterDuffBlendPipeline(options));

  auto dst_sampler_descriptor = descriptor_;
  dst_sampler_descriptor.width_address_mode =
      TileModeToAddressMode(tile_mode_x_, renderer.GetDeviceCapabilities())
          .value_or(SamplerAddressMode::kClampToEdge);
  dst_sampler_descriptor.height_address_mode =
      TileModeToAddressMode(tile_mode_y_, renderer.GetDeviceCapabilities())
          .value_or(SamplerAddressMode::kClampToEdge);

  if (texture_) {
    const std::unique_ptr<const Sampler>& dst_sampler =
        renderer.GetContext()->GetSamplerLibrary()->GetSampler(
            dst_sampler_descriptor);
    FS::BindTextureSamplerDst(pass, texture_, dst_sampler);
  }

  FS::FragInfo frag_info;
  VS::FrameInfo frame_info;

  frame_info.texture_sampler_y_coord_scale = texture_->GetYCoordScale();
  frag_info.output_alpha = alpha_;
  frag_info.input_alpha = 1.0;

  BlendMode blend_mode = BlendMode::kDestination;
  if (texture_) {
    blend_mode =
        InvertPorterDuffBlend(blend_mode_).value_or(BlendMode::kSource);
  }
  auto blend_coefficients =
      kPorterDuffCoefficients[static_cast<int>(blend_mode)];
  frag_info.src_coeff = blend_coefficients[0];
  frag_info.src_coeff_dst_alpha = blend_coefficients[1];
  frag_info.dst_coeff = blend_coefficients[2];
  frag_info.dst_coeff_src_alpha = blend_coefficients[3];
  frag_info.dst_coeff_src_color = blend_coefficients[4];

  auto& host_buffer = renderer.GetTransientsBuffer();
  FS::BindFragInfo(pass, host_buffer.EmplaceUniform(frag_info));

  frame_info.mvp = geometry_result.transform;

  auto uniform_view = host_buffer.EmplaceUniform(frame_info);
  VS::BindFrameInfo(pass, uniform_view);

  return pass.Draw().ok();
}

bool VerticesContents::RenderAdvanced(const ContentContext& renderer,
                                      const Entity& entity,
                                      RenderPass& pass) const {
  using VS = VerticesUberShader::VertexShader;
  using FS = VerticesUberShader::FragmentShader;

  Rect texture_coverage = (!!texture_) ? Rect::MakeSize(texture_->GetSize())
                                       : Rect::MakeLTRB(0, 0, 1, 1);
  GeometryResult geometry_result = geometry_->GetPositionUVColorBuffer(
      texture_coverage, inverse_matrix_, renderer, entity, pass);
  if (geometry_result.vertex_buffer.vertex_count == 0) {
    return true;
  }
  FML_DCHECK(geometry_result.mode == GeometryResult::Mode::kNormal);

#ifdef IMPELLER_DEBUG
  pass.SetCommandLabel(SPrintF("DrawVertices Advanced Blend (%s)",
                               BlendModeToString(blend_mode_)));
#endif  // IMPELLER_DEBUG
  pass.SetVertexBuffer(std::move(geometry_result.vertex_buffer));

  auto options = OptionsFromPassAndEntity(pass, entity);
  options.primitive_type = geometry_result.type;
  pass.SetPipeline(renderer.GetVerticesUberShader(options));

  auto dst_sampler_descriptor = descriptor_;
  dst_sampler_descriptor.width_address_mode =
      TileModeToAddressMode(tile_mode_x_, renderer.GetDeviceCapabilities())
          .value_or(SamplerAddressMode::kClampToEdge);
  dst_sampler_descriptor.height_address_mode =
      TileModeToAddressMode(tile_mode_y_, renderer.GetDeviceCapabilities())
          .value_or(SamplerAddressMode::kClampToEdge);

  if (texture_) {
    const std::unique_ptr<const Sampler>& dst_sampler =
        renderer.GetContext()->GetSamplerLibrary()->GetSampler(
            dst_sampler_descriptor);
    FS::BindTextureSampler(pass, texture_, dst_sampler);
  }

  FS::FragInfo frag_info;
  VS::FrameInfo frame_info;

  frame_info.texture_sampler_y_coord_scale = texture_->GetYCoordScale();
  frag_info.alpha = alpha_;
  frag_info.blend_mode = static_cast<int>(blend_mode_);

  auto& host_buffer = renderer.GetTransientsBuffer();
  FS::BindFragInfo(pass, host_buffer.EmplaceUniform(frag_info));

  frame_info.mvp = geometry_result.transform;

  auto uniform_view = host_buffer.EmplaceUniform(frame_info);
  VS::BindFrameInfo(pass, uniform_view);

  return pass.Draw().ok();
}

}  // namespace impeller
