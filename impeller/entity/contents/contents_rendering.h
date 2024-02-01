// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_CONTENTS_CONTENTS_RENDERING_H_
#define FLUTTER_IMPELLER_ENTITY_CONTENTS_CONTENTS_RENDERING_H_

#include <functional>
#include <memory>

#include "impeller/entity/contents/clip_contents.h"
#include "impeller/entity/contents/color_source_contents.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/geometry/geometry.h"
#include "impeller/renderer/pipeline.h"
#include "impeller/renderer/pipeline_descriptor.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

using BindFragmentCallback = std::function<bool(RenderPass& pass)>;
using PipelineBuilderMethod = std::shared_ptr<Pipeline<PipelineDescriptor>> (
    impeller::ContentContext::*)(ContentContextOptions) const;

template <typename VertexShaderT>
bool DrawGeometry(GeometryResult geometry_result,
                  const ColorSourceContents& contents,
                  const ContentContext& renderer,
                  const Entity& entity,
                  RenderPass& pass,
                  const PipelineBuilderMethod& pipeline_builder,
                  typename VertexShaderT::FrameInfo frame_info,
                  const BindFragmentCallback& bind_pipeline_callback) {
  auto options = OptionsFromPassAndEntity(pass, entity);

  // If overdraw prevention is enabled (like when drawing stroke paths), we
  // increment the stencil buffer as we draw, preventing overlapping fragments
  // from drawing. Afterwards, we need to append another draw call to clean up
  // the stencil buffer (happens below in this method).
  if (geometry_result.prevent_overdraw) {
    options.stencil_mode =
        ContentContextOptions::StencilMode::kLegacyClipIncrement;
  }
  options.primitive_type = geometry_result.type;
  pass.SetVertexBuffer(std::move(geometry_result.vertex_buffer));
  pass.SetStencilReference(entity.GetClipDepth());

  frame_info.depth = entity.GetShaderClipDepth();
  frame_info.mvp = geometry_result.transform;

  VertexShaderT::BindFrameInfo(
      pass, renderer.GetTransientsBuffer().EmplaceUniform(frame_info));

  std::shared_ptr<Pipeline<PipelineDescriptor>> pipeline =
      (renderer.*pipeline_builder)(options);
  pass.SetPipeline(pipeline);

  if (!bind_pipeline_callback(pass)) {
    return false;
  }

  if (!pass.Draw().ok()) {
    return false;
  }

  // If we performed overdraw prevention, a subsection of the clip heightmap was
  // incremented by 1 in order to self-clip. So simply append a clip restore to
  // clean it up.
  if (geometry_result.prevent_overdraw) {
    auto restore = ClipRestoreContents();
    restore.SetRestoreCoverage(contents.GetCoverage(entity));
    return restore.Render(renderer, entity, pass);
  }
  return true;
}

template <typename VertexShaderT>
bool DrawPositions(const ColorSourceContents& contents,
                   const ContentContext& renderer,
                   const Entity& entity,
                   RenderPass& pass,
                   const PipelineBuilderMethod& pipeline_builder,
                   typename VertexShaderT::FrameInfo frame_info,
                   const BindFragmentCallback& bind_pipeline_callback) {
  auto geometry_result =
      contents.GetGeometry()->GetPositionBuffer(renderer, entity, pass);

  return DrawGeometry<VertexShaderT>(geometry_result, contents, renderer,
                                     entity, pass, pipeline_builder, frame_info,
                                     bind_pipeline_callback);
}

template <typename VertexShaderT>
bool DrawPositionsAndUVs(Rect texture_coverage,
                         Matrix effect_transform,
                         const ColorSourceContents& contents,
                         const ContentContext& renderer,
                         const Entity& entity,
                         RenderPass& pass,
                         const PipelineBuilderMethod& pipeline_builder,
                         typename VertexShaderT::FrameInfo frame_info,
                         const BindFragmentCallback& bind_pipeline_callback) {
  auto geometry_result = contents.GetGeometry()->GetPositionUVBuffer(
      texture_coverage, effect_transform, renderer, entity, pass);

  return DrawGeometry<VertexShaderT>(geometry_result, contents, renderer,
                                     entity, pass, pipeline_builder, frame_info,
                                     bind_pipeline_callback);
}

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_CONTENTS_CONTENTS_RENDERING_H_
