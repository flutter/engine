// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "framebuffer_blend_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/geometry/color.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

FramebufferBlendContents::FramebufferBlendContents() = default;

FramebufferBlendContents::~FramebufferBlendContents() = default;

void FramebufferBlendContents::SetBlendMode(BlendMode blend_mode) {
  blend_mode_ = blend_mode;
}

// |Contents|
std::optional<Rect> FramebufferBlendContents::GetCoverage(
    const Entity& entity) const {
  // dst rect is already transformed.
  return dest_rect_;
}

bool FramebufferBlendContents::Render(const ContentContext& renderer,
                                      const Entity& entity,
                                      RenderPass& pass) const {
  if (!renderer.GetDeviceCapabilities().SupportsFramebufferFetch()) {
    return false;
  }

  using VS = ScratchSpaceBlendScreenPipeline::VertexShader;

  auto& host_buffer = renderer.GetTransientsBuffer();

  VertexBufferBuilder<VS::PerVertexData> vtx_builder;
  vtx_builder.AddVertices({
      {dest_rect_.GetLeftTop(),},
      {dest_rect_.GetRightTop()},
      {dest_rect_.GetLeftBottom()},
      {dest_rect_.GetRightBottom()},
  });

  auto options = OptionsFromPass(pass);
  options.blend_mode = BlendMode::kSource;
  if (blend_mode_ == BlendMode::kSourceOver) {
    options.blend_mode = BlendMode::kSourceOver;
  }
  options.primitive_type = PrimitiveType::kTriangleStrip;
  options.scratch_flush = true;

  pass.SetCommandLabel("Scratch Space Blend");
  pass.SetVertexBuffer(vtx_builder.CreateVertexBuffer(host_buffer));

  switch (blend_mode_) {
    case BlendMode::kScreen:
      pass.SetPipeline(renderer.GetScratchSpaceBlendScreenPipeline(options));
      break;
    case BlendMode::kOverlay:
      pass.SetPipeline(renderer.GetScratchSpaceBlendOverlayPipeline(options));
      break;
    case BlendMode::kDarken:
      pass.SetPipeline(renderer.GetScratchSpaceBlendDarkenPipeline(options));
      break;
    case BlendMode::kLighten:
      pass.SetPipeline(renderer.GetScratchSpaceBlendLightenPipeline(options));
      break;
    case BlendMode::kColorDodge:
      pass.SetPipeline(renderer.GetScratchSpaceBlendColorDodgePipeline(options));
      break;
    case BlendMode::kColorBurn:
      pass.SetPipeline(renderer.GetScratchSpaceBlendColorBurnPipeline(options));
      break;
    case BlendMode::kHardLight:
      pass.SetPipeline(renderer.GetScratchSpaceBlendHardLightPipeline(options));
      break;
    case BlendMode::kSoftLight:
      pass.SetPipeline(renderer.GetScratchSpaceBlendSoftLightPipeline(options));
      break;
    case BlendMode::kDifference:
      pass.SetPipeline(renderer.GetScratchSpaceBlendDifferencePipeline(options));
      break;
    case BlendMode::kExclusion:
      pass.SetPipeline(renderer.GetScratchSpaceBlendExclusionPipeline(options));
      break;
    case BlendMode::kMultiply:
      pass.SetPipeline(renderer.GetScratchSpaceBlendMultiplyPipeline(options));
      break;
    case BlendMode::kHue:
      pass.SetPipeline(renderer.GetScratchSpaceBlendHuePipeline(options));
      break;
    case BlendMode::kSaturation:
      pass.SetPipeline(renderer.GetScratchSpaceBlendSaturationPipeline(options));
      break;
    case BlendMode::kColor:
      pass.SetPipeline(renderer.GetScratchSpaceBlendColorPipeline(options));
      break;
    case BlendMode::kLuminosity:
      pass.SetPipeline(renderer.GetScratchSpaceBlendLuminosityPipeline(options));
      break;
    default:
      pass.SetPipeline(renderer.GetScratchSpaceFlush(options));
      break;
  }

  VS::FrameInfo frame_info;

  frame_info.mvp = Entity::GetShaderTransform(entity.GetShaderClipDepth(), pass,
                                              entity.GetTransform()); // Already transformed
  VS::BindFrameInfo(pass, host_buffer.EmplaceUniform(frame_info));

  if (blend_mode_ == BlendMode::kSourceOver) {
    using FS = ScratchSpaceFlushPipeline::FragmentShader;
    FS::FragInfo frag_info;
    frag_info.alpha = alpha_;

    FS::BindFragInfo(pass, host_buffer.EmplaceUniform(frag_info));
  }

  return pass.Draw().ok();
}

}  // namespace impeller
