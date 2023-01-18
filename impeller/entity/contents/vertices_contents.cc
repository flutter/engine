// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vertices_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/filters/color_filter_contents.h"
#include "impeller/entity/contents/filters/filter_contents.h"
#include "impeller/entity/position.vert.h"
#include "impeller/entity/position_color.vert.h"
#include "impeller/entity/position_uv.vert.h"
#include "impeller/entity/vertices.frag.h"
#include "impeller/geometry/color.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"
#include "impeller/renderer/vertex_buffer.h"

namespace impeller {

VerticesContents::VerticesContents() = default;

VerticesContents::~VerticesContents() = default;

std::optional<Rect> VerticesContents::GetCoverage(const Entity& entity) const {
  return geometry_->GetCoverage(entity.GetTransformation());
};

void VerticesContents::SetGeometry(std::unique_ptr<VerticesGeometry> geometry) {
  geometry_ = std::move(geometry);
}

void VerticesContents::SetSourceContents(std::shared_ptr<Contents> contents) {
  src_contents_ = std::move(contents);
}

void VerticesContents::SetColor(Color color) {
  // Note: for blending the fully opaque color is used, and the alpha is
  // applied as a final step.
  color_ = color;
}

void VerticesContents::SetBlendMode(BlendMode blend_mode) {
  blend_mode_ = blend_mode;
}

bool VerticesContents::Render(const ContentContext& renderer,
                              const Entity& entity,
                              RenderPass& pass) const {
  if (geometry_->HasVertexColors() && blend_mode_ == BlendMode::kClear) {
    return true;
  }
  if (geometry_->HasVertexColors() && blend_mode_ == BlendMode::kDestination) {
    return RenderColors(renderer, entity, pass, color_.alpha);
  }
  if (!geometry_->HasVertexColors() || blend_mode_ == BlendMode::kSource) {
    return src_contents_->Render(renderer, entity, pass);
  }

  auto coverage =
      geometry_->GetCoverage(Matrix()).value_or(Rect::MakeLTRB(0, 0, 0, 0));
  auto size = coverage.size;
  if (size.IsEmpty()) {
    return true;
  }

  auto dst_texture = renderer.MakeSubpass(
      ISize::Ceil(size),
      [&contents = *this, &coverage](const ContentContext& renderer,
                                     RenderPass& pass) -> bool {
        Entity sub_entity;
        sub_entity.SetBlendMode(BlendMode::kSourceOver);
        sub_entity.SetTransformation(
            Matrix::MakeTranslation(Vector3(-coverage.origin)));
        return contents.RenderColors(renderer, sub_entity, pass, 1.0);
      });

  if (!dst_texture) {
    return false;
  }
  auto contents = ColorFilterContents::MakeBlend(
      blend_mode_,
      {FilterInput::Make(src_contents_), FilterInput::Make(dst_texture)});
  contents->SetAlpha(color_.alpha);
  return contents->Render(renderer, entity, pass);
}

bool VerticesContents::RenderColors(const ContentContext& renderer,
                                    const Entity& entity,
                                    RenderPass& pass,
                                    Scalar alpha) const {
  using VS = GeometryColorPipeline::VertexShader;
  using FS = GeometryColorPipeline::FragmentShader;

  Command cmd;
  cmd.label = "VerticesColors";
  auto& host_buffer = pass.GetTransientsBuffer();

  auto geometry_result = geometry_->GetPositionColorBuffer(
      renderer, entity, pass, color_, blend_mode_);
  auto opts = OptionsFromPassAndEntity(pass, entity);
  opts.primitive_type = geometry_result.type;
  cmd.pipeline = renderer.GetGeometryColorPipeline(opts);
  cmd.BindVertices(geometry_result.vertex_buffer);

  VS::VertInfo vert_info;
  vert_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                  entity.GetTransformation();
  VS::BindVertInfo(cmd, host_buffer.EmplaceUniform(vert_info));

  FS::FragInfo frag_info;
  frag_info.alpha = alpha;
  FS::BindFragInfo(cmd, host_buffer.EmplaceUniform(frag_info));

  return pass.AddCommand(std::move(cmd));
}

}  // namespace impeller
