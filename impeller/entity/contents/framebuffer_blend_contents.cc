// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "framebuffer_blend_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/framebuffer_blend_context.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"

namespace impeller {

FramebufferBlendContents::FramebufferBlendContents() = default;

FramebufferBlendContents::~FramebufferBlendContents() = default;

void FramebufferBlendContents::SetBlendMode(BlendMode blend_mode) {
  blend_mode_ = blend_mode;
}

void FramebufferBlendContents::SetChildContents(
    std::shared_ptr<Contents> child_contents) {
  child_contents_ = std::move(child_contents);
}

// |Contents|
std::optional<Rect> FramebufferBlendContents::GetCoverage(
    const Entity& entity) const {
  return child_contents_->GetCoverage(entity);
}

bool FramebufferBlendContents::Render(const ContentContext& renderer,
                                      const Entity& entity,
                                      RenderPass& pass) const {
  if (!renderer.GetDeviceCapabilities().SupportsFramebufferBlending()) {
    return false;
  }
  // TODO(jonahwilliams): get these shaders compiling more generally.
#if FML_OS_PHYSICAL_IOS
  using VS = FramebufferBlendScreenPipeline::VertexShader;
  using FS = FramebufferBlendScreenPipeline::FragmentShader;

  auto& host_buffer = pass.GetTransientsBuffer();

  auto src_snapshot = child_contents_->RenderToSnapshot(renderer, entity);
  if (!src_snapshot.has_value()) {
    return true;
  }
  auto coverage = src_snapshot->GetCoverage();
  if (!coverage.has_value()) {
    return true;
  }
  Rect src_coverage = coverage.value();
  auto maybe_src_uvs = src_snapshot->GetCoverageUVs(src_coverage);
  if (!maybe_src_uvs.has_value()) {
    return true;
  }
  std::array<Point, 4> src_uvs = maybe_src_uvs.value();

  auto size = src_coverage.size;
  VertexBufferBuilder<VS::PerVertexData> vtx_builder;
  vtx_builder.AddVertices({
      {Point(0, 0), src_uvs[0]},
      {Point(size.width, 0), src_uvs[1]},
      {Point(size.width, size.height), src_uvs[3]},
      {Point(0, 0), src_uvs[0]},
      {Point(size.width, size.height), src_uvs[3]},
      {Point(0, size.height), src_uvs[2]},
  });
  auto vtx_buffer = vtx_builder.CreateVertexBuffer(host_buffer);

  auto options = OptionsFromPass(pass);
  options.blend_mode = BlendMode::kSource;

  Command cmd;
  cmd.label = "Framebuffer Advanced Blend Filter";
  cmd.BindVertices(vtx_buffer);
  cmd.stencil_reference = entity.GetStencilDepth();

  const auto ctx = renderer.GetFramebufferBlendContext();
  switch (blend_mode_) {
    case BlendMode::kScreen:
      cmd.pipeline = ctx->GetFramebufferBlendScreenPipeline(options);
      break;
    case BlendMode::kOverlay:
      cmd.pipeline = ctx->GetFramebufferBlendOverlayPipeline(options);
      break;
    case BlendMode::kDarken:
      cmd.pipeline = ctx->GetFramebufferBlendDarkenPipeline(options);
      break;
    case BlendMode::kLighten:
      cmd.pipeline = ctx->GetFramebufferBlendLightenPipeline(options);
      break;
    case BlendMode::kColorDodge:
      cmd.pipeline = ctx->GetFramebufferBlendColorDodgePipeline(options);
      break;
    case BlendMode::kColorBurn:
      cmd.pipeline = ctx->GetFramebufferBlendColorBurnPipeline(options);
      break;
    case BlendMode::kHardLight:
      cmd.pipeline = ctx->GetFramebufferBlendHardLightPipeline(options);
      break;
    case BlendMode::kSoftLight:
      cmd.pipeline = ctx->GetFramebufferBlendSoftLightPipeline(options);
      break;
    case BlendMode::kDifference:
      cmd.pipeline = ctx->GetFramebufferBlendDifferencePipeline(options);
      break;
    case BlendMode::kExclusion:
      cmd.pipeline = ctx->GetFramebufferBlendExclusionPipeline(options);
      break;
    case BlendMode::kMultiply:
      cmd.pipeline = ctx->GetFramebufferBlendMultiplyPipeline(options);
      break;
    case BlendMode::kHue:
      cmd.pipeline = ctx->GetFramebufferBlendHuePipeline(options);
      break;
    case BlendMode::kSaturation:
      cmd.pipeline = ctx->GetFramebufferBlendSaturationPipeline(options);
      break;
    case BlendMode::kColor:
      cmd.pipeline = ctx->GetFramebufferBlendColorPipeline(options);
      break;
    case BlendMode::kLuminosity:
      cmd.pipeline = ctx->GetFramebufferBlendLuminosityPipeline(options);
      break;
    default:
      return false;
  }

  VS::FrameInfo frame_info;

  auto src_sampler = renderer.GetContext()->GetSamplerLibrary()->GetSampler(
      src_snapshot->sampler_descriptor);
  FS::BindTextureSamplerSrc(cmd, src_snapshot->texture, src_sampler);

  frame_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   src_snapshot->transform;
  frame_info.src_y_coord_scale = src_snapshot->texture->GetYCoordScale();

  auto uniform_view = host_buffer.EmplaceUniform(frame_info);
  VS::BindFrameInfo(cmd, uniform_view);

  return pass.AddCommand(cmd);
#else
  return false;
#endif  // FML_OS_PHYSICAL_IOS
}

}  // namespace impeller
