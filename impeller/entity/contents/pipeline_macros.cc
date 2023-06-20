// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/pipeline_macros.h"

#include "impeller/base/validation.h"

namespace impeller {

void ContentContextOptions::ApplyToPipelineDescriptor(
    PipelineDescriptor& desc) const {
  auto pipeline_blend = blend_mode;
  if (blend_mode > Entity::kLastPipelineBlendMode) {
    VALIDATION_LOG << "Cannot use blend mode " << static_cast<int>(blend_mode)
                   << " as a pipeline blend.";
    pipeline_blend = BlendMode::kSourceOver;
  }

  desc.SetSampleCount(sample_count);

  ColorAttachmentDescriptor color0 = *desc.GetColorAttachmentDescriptor(0u);
  color0.format = color_attachment_pixel_format.value_or(PixelFormat::kUnknown);
  color0.alpha_blend_op = BlendOperation::kAdd;
  color0.color_blend_op = BlendOperation::kAdd;

  switch (pipeline_blend) {
    case BlendMode::kClear:
      color0.dst_alpha_blend_factor = BlendFactor::kZero;
      color0.dst_color_blend_factor = BlendFactor::kZero;
      color0.src_alpha_blend_factor = BlendFactor::kZero;
      color0.src_color_blend_factor = BlendFactor::kZero;
      break;
    case BlendMode::kSource:
      color0.dst_alpha_blend_factor = BlendFactor::kZero;
      color0.dst_color_blend_factor = BlendFactor::kZero;
      color0.src_alpha_blend_factor = BlendFactor::kOne;
      color0.src_color_blend_factor = BlendFactor::kOne;
      break;
    case BlendMode::kDestination:
      color0.dst_alpha_blend_factor = BlendFactor::kOne;
      color0.dst_color_blend_factor = BlendFactor::kOne;
      color0.src_alpha_blend_factor = BlendFactor::kZero;
      color0.src_color_blend_factor = BlendFactor::kZero;
      break;
    case BlendMode::kSourceOver:
      color0.dst_alpha_blend_factor = BlendFactor::kOneMinusSourceAlpha;
      color0.dst_color_blend_factor = BlendFactor::kOneMinusSourceAlpha;
      color0.src_alpha_blend_factor = BlendFactor::kOne;
      color0.src_color_blend_factor = BlendFactor::kOne;
      break;
    case BlendMode::kDestinationOver:
      color0.dst_alpha_blend_factor = BlendFactor::kOne;
      color0.dst_color_blend_factor = BlendFactor::kOne;
      color0.src_alpha_blend_factor = BlendFactor::kOneMinusDestinationAlpha;
      color0.src_color_blend_factor = BlendFactor::kOneMinusDestinationAlpha;
      break;
    case BlendMode::kSourceIn:
      color0.dst_alpha_blend_factor = BlendFactor::kZero;
      color0.dst_color_blend_factor = BlendFactor::kZero;
      color0.src_alpha_blend_factor = BlendFactor::kDestinationAlpha;
      color0.src_color_blend_factor = BlendFactor::kDestinationAlpha;
      break;
    case BlendMode::kDestinationIn:
      color0.dst_alpha_blend_factor = BlendFactor::kSourceAlpha;
      color0.dst_color_blend_factor = BlendFactor::kSourceAlpha;
      color0.src_alpha_blend_factor = BlendFactor::kZero;
      color0.src_color_blend_factor = BlendFactor::kZero;
      break;
    case BlendMode::kSourceOut:
      color0.dst_alpha_blend_factor = BlendFactor::kZero;
      color0.dst_color_blend_factor = BlendFactor::kZero;
      color0.src_alpha_blend_factor = BlendFactor::kOneMinusDestinationAlpha;
      color0.src_color_blend_factor = BlendFactor::kOneMinusDestinationAlpha;
      break;
    case BlendMode::kDestinationOut:
      color0.dst_alpha_blend_factor = BlendFactor::kOneMinusSourceAlpha;
      color0.dst_color_blend_factor = BlendFactor::kOneMinusSourceAlpha;
      color0.src_alpha_blend_factor = BlendFactor::kZero;
      color0.src_color_blend_factor = BlendFactor::kZero;
      break;
    case BlendMode::kSourceATop:
      color0.dst_alpha_blend_factor = BlendFactor::kOneMinusSourceAlpha;
      color0.dst_color_blend_factor = BlendFactor::kOneMinusSourceAlpha;
      color0.src_alpha_blend_factor = BlendFactor::kDestinationAlpha;
      color0.src_color_blend_factor = BlendFactor::kDestinationAlpha;
      break;
    case BlendMode::kDestinationATop:
      color0.dst_alpha_blend_factor = BlendFactor::kSourceAlpha;
      color0.dst_color_blend_factor = BlendFactor::kSourceAlpha;
      color0.src_alpha_blend_factor = BlendFactor::kOneMinusDestinationAlpha;
      color0.src_color_blend_factor = BlendFactor::kOneMinusDestinationAlpha;
      break;
    case BlendMode::kXor:
      color0.dst_alpha_blend_factor = BlendFactor::kOneMinusSourceAlpha;
      color0.dst_color_blend_factor = BlendFactor::kOneMinusSourceAlpha;
      color0.src_alpha_blend_factor = BlendFactor::kOneMinusDestinationAlpha;
      color0.src_color_blend_factor = BlendFactor::kOneMinusDestinationAlpha;
      break;
    case BlendMode::kPlus:
      color0.dst_alpha_blend_factor = BlendFactor::kOne;
      color0.dst_color_blend_factor = BlendFactor::kOne;
      color0.src_alpha_blend_factor = BlendFactor::kOne;
      color0.src_color_blend_factor = BlendFactor::kOne;
      break;
    case BlendMode::kModulate:
      color0.dst_alpha_blend_factor = BlendFactor::kSourceAlpha;
      color0.dst_color_blend_factor = BlendFactor::kSourceColor;
      color0.src_alpha_blend_factor = BlendFactor::kZero;
      color0.src_color_blend_factor = BlendFactor::kZero;
      break;
    default:
      FML_UNREACHABLE();
  }
  desc.SetColorAttachmentDescriptor(0u, color0);

  if (!has_stencil_attachment) {
    desc.ClearStencilAttachments();
  }

  auto maybe_stencil = desc.GetFrontStencilAttachmentDescriptor();
  if (maybe_stencil.has_value()) {
    StencilAttachmentDescriptor stencil = maybe_stencil.value();
    stencil.stencil_compare = stencil_compare;
    stencil.depth_stencil_pass = stencil_operation;
    desc.SetStencilAttachmentDescriptors(stencil);
  }

  desc.SetPrimitiveType(primitive_type);
  desc.SetPolygonMode(wireframe ? PolygonMode::kLine : PolygonMode::kFill);
}

}  // namespace impeller
