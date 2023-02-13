// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "framebuffer_blend_contents.h"

namespace impeller {

FramebufferBlendContents::FramebufferBlendContents() = default;

FramebufferBlendContents::~FramebufferBlendContents() = default;

void FramebufferBlendContents::SetBlendMode(BlendMode blend_mode) {
  blend_mode_ = blend_mode;
}

void FramebufferBlendContents::SetForegroundColor(Color color) {
  foreground_color_ = color;
}

void FramebufferBlendContents::SetChildContents(
    std::shared_ptr<Contents> child_contents) {
  child_contents_ = std::move(child_contents);
}

bool FramebufferBlendContents::Render(const ContentContext& renderer,
                                      const Entity& entity,
                                      RenderPass& pass) const {
  using VS = FramebufferBlendScreenPipeline::VertexShader;
  using FS = FramebufferBlendScreenPipeline::FragmentShader;

  auto& host_buffer = pass.GetTransientsBuffer();

  std::optional<Snapshot> src_snapshot;
  std::array<Point, 4> src_uvs;
  Rect src_coverage;

  if (!foreground_color_.has_value()) {
    src_snapshot = child_contents_->RenderToSnapshot(renderer, entity);
    if (!src_snapshot.has_value()) {
      return std::nullopt;
    }
    auto maybe_src_uvs = src_snapshot->GetCoverageUVs(coverage);
    if (!maybe_src_uvs.has_value()) {
      return std::nullopt;
    }
    src_uvs = maybe_src_uvs.value();
    auto coverage = src_snapshot->GetCoverage();
    if (!coverage.has_value()) {
      return std::nullopt;
    }
    src_coverage = coverage;
  }

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
  std::shared_ptr<Pipeline<PipelineDescriptor>> pipeline =
      std::invoke(pipeline_proc, renderer, options);

  Command cmd;
  cmd.label = "Framebuffer Advanced Blend Filter";
  cmd.BindVertices(vtx_buffer);

  switch (blend_mode_) {
    case BlendMode::kScreen:
      cmd.pipeline = renderer.GetFramebufferBlendScreenPipeline();
      break;
    case BlendMode::kOverlay:
      cmd.pipeline = renderer.GetFramebufferBlendOverlayPipeline();
      break;
    case BlendMode::kDarken:
      cmd.pipeline = renderer.GetFramebufferBlendDarkenPipeline();
      break;
    case BlendMode::kLighten:
      cmd.pipeline = renderer.GetFramebufferBlendLightenPipeline();
      break;
    case BlendMode::kColorDodge:
      cmd.pipeline = renderer.GetFramebufferBlendColorDidgePipeline();
      break;
    case BlendMode::kColorBurn:
      cmd.pipeline = renderer.GetFramebufferBlendColorBurnPipeline();
      break;
    case BlendMode::kHardLight:
      cmd.pipeline = renderer.GetFramebufferBlendHardlightPipeline();
      break;
    case BlendMode::kSoftLight:
      cmd.pipeline = renderer.GetFramebufferBlendSoftlightPipeline();
      break;
    case BlendMode::kDifference:
      cmd.pipeline = renderer.GetFramebufferBlendDifferencePipeline();
      break;
    case BlendMode::kExclusion:
      cmd.pipeline = renderer.GetFramebufferBlendExclusionPipeline();
      break;
    case BlendMode::kMultiply:
      cmd.pipeline = renderer.GetFramebufferBlendMultiplyPipeline();
      break;
    case BlendMode::kHue:
      cmd.pipeline = renderer.GetFramebufferBlendHuePipeline();
      break;
    case BlendMode::kSaturation:
      cmd.pipeline = renderer.GetFramebufferBlendSaturationPipeline();
      break;
    case BlendMode::kColor:
      cmd.pipeline = renderer.GetFramebufferBlendColorPipeline();
      break;
    case BlendMode::kLuminosity:
      cmd.pipeline = renderer.GetFramebufferBlendLuminosityPipeline();
      break;
  }

  FS::BlendInfo blend_info;

  if (foreground_color.has_value()) {
    blend_info.color_factor = 1;
    blend_info.color = foreground_color.value();
  } else {
    auto src_sampler = renderer.GetContext()->GetSamplerLibrary()->GetSampler(
        src_snapshot->sampler_descriptor);
    blend_info.color_factor = 0;
    FS::BindTextureSamplerSrc(cmd, src_snapshot->texture, src_sampler);
    blend_info.src_y_coord_scale = src_snapshot->texture->GetYCoordScale();
  }

  auto blend_uniform = host_buffer.EmplaceUniform(blend_info);
  FS::BindBlendInfo(cmd, blend_uniform);

  frame_info.mvp = Matrix::MakeOrthographic(size);

  auto uniform_view = host_buffer.EmplaceUniform(frame_info);
  VS::BindFrameInfo(cmd, uniform_view);

  return pass.AddCommand(cmd);
}

}  // namespace impeller