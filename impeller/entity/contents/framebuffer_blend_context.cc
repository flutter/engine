// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/framebuffer_blend_context.h"

#include <iostream>
#include "impeller/entity/entity.h"
namespace impeller {

template <typename PipelineT>
static std::unique_ptr<PipelineT> CreateDefaultPipeline(
    const Context& context) {
  auto desc = PipelineT::Builder::MakeDefaultPipelineDescriptor(context);
  if (!desc.has_value()) {
    return nullptr;
  }
  // Apply default ContentContextOptions to the descriptor.
  const auto default_color_fmt = context.GetColorAttachmentPixelFormat();
  ContentContextOptions{.color_attachment_pixel_format = default_color_fmt}
      .ApplyToPipelineDescriptor(*desc);
  return std::make_unique<PipelineT>(context, desc);
}

FramebufferBlendContext::FramebufferBlendContext(
    std::shared_ptr<Context> context)
    : context_(std::move(context)) {
  if (!context_ || !context_->IsValid()) {
    return;
  }

  if (context_->GetDeviceCapabilities().SupportsFramebufferBlending()) {
    framebuffer_blend_color_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendColorPipeline>(*context_);
    framebuffer_blend_colorburn_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendColorBurnPipeline>(*context_);
    framebuffer_blend_colordodge_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendColorDodgePipeline>(*context_);
    framebuffer_blend_darken_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendDarkenPipeline>(*context_);
    framebuffer_blend_difference_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendDifferencePipeline>(*context_);
    framebuffer_blend_exclusion_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendExclusionPipeline>(*context_);
    framebuffer_blend_hardlight_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendHardLightPipeline>(*context_);
    framebuffer_blend_hue_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendHuePipeline>(*context_);
    framebuffer_blend_lighten_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendLightenPipeline>(*context_);
    framebuffer_blend_luminosity_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendLuminosityPipeline>(*context_);
    framebuffer_blend_multiply_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendMultiplyPipeline>(*context_);
    framebuffer_blend_overlay_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendOverlayPipeline>(*context_);
    framebuffer_blend_saturation_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendSaturationPipeline>(*context_);
    framebuffer_blend_screen_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendScreenPipeline>(*context_);
    framebuffer_blend_softlight_pipelines_[{}] =
        CreateDefaultPipeline<FramebufferBlendSoftLightPipeline>(*context_);
  }
  is_valid_ = true;
}

FramebufferBlendContext::~FramebufferBlendContext() = default;

bool FramebufferBlendContext::IsValid() const {
  return is_valid_;
}

}  // namespace impeller