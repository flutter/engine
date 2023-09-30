// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "solid_color_contents.h"

#include "impeller/core/formats.h"
#include "impeller/entity/contents/clip_contents.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/geometry/geometry.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/path.h"
#include "impeller/renderer/pipeline_descriptor.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

SolidColorContents::SolidColorContents() = default;

SolidColorContents::~SolidColorContents() = default;

void SolidColorContents::SetColor(Color color) {
  color_ = color;
}

Color SolidColorContents::GetColor() const {
  return color_.WithAlpha(color_.alpha * GetOpacityFactor());
}

bool SolidColorContents::IsSolidColor() const {
  return true;
}

bool SolidColorContents::IsOpaque() const {
  return GetColor().IsOpaque();
}

std::optional<Rect> SolidColorContents::GetCoverage(
    const Entity& entity) const {
  if (GetColor().IsTransparent()) {
    return std::nullopt;
  }

  auto geometry = GetGeometry();
  if (geometry == nullptr) {
    return std::nullopt;
  }
  return geometry->GetCoverage(entity.GetTransformation());
};

bool SolidColorContents::Render(const ContentContext& renderer,
                                const Entity& entity,
                                RenderPass& pass) const {
  if (GetGeometry()->UseStcHint()) {
    return RenderSTC(renderer, entity, pass);
  }
  auto capture = entity.GetCapture().CreateChild("SolidColorContents");

  using VS = SolidFillPipeline::VertexShader;

  Command cmd;
  DEBUG_COMMAND_INFO(cmd, "Solid Fill");
  cmd.stencil_reference = entity.GetStencilDepth();

  auto geometry_result =
      GetGeometry()->GetPositionBuffer(renderer, entity, pass);

  auto options = OptionsFromPassAndEntity(pass, entity);
  if (geometry_result.prevent_overdraw) {
    options.stencil_compare = CompareFunction::kEqual;
    options.stencil_operation = StencilOperation::kIncrementClamp;
  }

  options.primitive_type = geometry_result.type;
  cmd.pipeline = renderer.GetSolidFillPipeline(options);
  cmd.BindVertices(geometry_result.vertex_buffer);

  VS::FrameInfo frame_info;
  frame_info.mvp = capture.AddMatrix("Transform", geometry_result.transform);
  frame_info.color = capture.AddColor("Color", GetColor()).Premultiply();
  VS::BindFrameInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(frame_info));

  if (!pass.AddCommand(std::move(cmd))) {
    return false;
  }

  if (geometry_result.prevent_overdraw) {
    auto restore = ClipRestoreContents();
    restore.SetRestoreCoverage(GetCoverage(entity));
    return restore.Render(renderer, entity, pass);
  }
  return true;
}

bool SolidColorContents::RenderSTC(const ContentContext& renderer,
                                   const Entity& entity,
                                   RenderPass& pass) const {
  // http://www.glprogramming.com/red/chapter14.html#name13

  {
    using VS = ClipPipeline::VertexShader;
    // Part 1, Stencil.
    Command cmd;
    DEBUG_COMMAND_INFO(cmd, "STC Fill");

    auto geometry_result =
        GetGeometry()->GetPositionBuffer(renderer, entity, pass, true);

    auto options = OptionsFromPassAndEntity(pass, entity);
    options.stencil_compare = CompareFunction::kAlways;
    options.stencil_operation = StencilOperation::kInvert;
    options.blend_mode = BlendMode::kDestination;

    options.primitive_type = geometry_result.type;
    cmd.pipeline = renderer.GetClipPipeline(options);
    cmd.BindVertices(geometry_result.vertex_buffer);
    cmd.stencil_reference = 0;

    VS::FrameInfo frame_info;
    frame_info.mvp = geometry_result.transform;
    VS::BindFrameInfo(cmd,
                      pass.GetTransientsBuffer().EmplaceUniform(frame_info));

    if (!pass.AddCommand(std::move(cmd))) {
      return false;
    }
  }

  {
    using VS = SolidFillPipeline::VertexShader;
    // Part 2, Cover.
    Command cmd;
    DEBUG_COMMAND_INFO(cmd, "Solid Fill");

    auto ident = Matrix();
    auto geometry_result =
        Geometry::MakeRect(GetGeometry()->GetCoverage(ident).value())
            ->GetPositionBuffer(renderer, entity, pass);

    auto options = OptionsFromPassAndEntity(pass, entity);
    options.stencil_compare = CompareFunction::kNotEqual;
    options.stencil_operation = StencilOperation::kSetToReferenceValue;

    options.primitive_type = geometry_result.type;
    cmd.pipeline = renderer.GetSolidFillPipeline(options);
    cmd.BindVertices(geometry_result.vertex_buffer);
    cmd.stencil_reference = 0;

    VS::FrameInfo frame_info;
    frame_info.mvp = geometry_result.transform;
    frame_info.color = GetColor().Premultiply();
    VS::BindFrameInfo(cmd,
                      pass.GetTransientsBuffer().EmplaceUniform(frame_info));

    if (!pass.AddCommand(std::move(cmd))) {
      return false;
    }
  }

  return true;
}

std::unique_ptr<SolidColorContents> SolidColorContents::Make(const Path& path,
                                                             Color color) {
  auto contents = std::make_unique<SolidColorContents>();
  contents->SetGeometry(Geometry::MakeFillPath(path));
  contents->SetColor(color);
  return contents;
}

std::optional<Color> SolidColorContents::AsBackgroundColor(
    const Entity& entity,
    ISize target_size) const {
  Rect target_rect = Rect::MakeSize(target_size);
  return GetGeometry()->CoversArea(entity.GetTransformation(), target_rect)
             ? GetColor()
             : std::optional<Color>();
}

bool SolidColorContents::ApplyColorFilter(
    const ColorFilterProc& color_filter_proc) {
  color_ = color_filter_proc(color_);
  return true;
}

}  // namespace impeller
