// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/linear_to_srgb_filter_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/geometry/point.h"
#include "impeller/geometry/vector.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"

namespace impeller {

LinearToSrgbFilterContents::LinearToSrgbFilterContents() = default;

LinearToSrgbFilterContents::~LinearToSrgbFilterContents() = default;

bool LinearToSrgbFilterContents::RenderFilter(const FilterInput::Vector& inputs,
                                              const ContentContext& renderer,
                                              const Entity& entity,
                                              RenderPass& pass,
                                              const Rect& coverage) const {
  if (inputs.empty()) {
    return true;
  }

  using VS = LinearToSrgbFilterPipeline::VertexShader;
  using FS = LinearToSrgbFilterPipeline::FragmentShader;

  auto input_snapshot = inputs[0]->GetSnapshot(renderer, entity);
  if (!input_snapshot.has_value()) {
    return true;
  }

  Command cmd;
  cmd.label = "Linear to sRGB Filter";

  auto options = OptionsFromPass(pass);
  options.blend_mode = Entity::BlendMode::kSource;
  cmd.pipeline = renderer.GetLinearToSrgbFilterPipeline(options);

  VertexBufferBuilder<VS::PerVertexData> vtx_builder;
  vtx_builder.AddVertices({
      {Point(0, 0)},
      {Point(1, 0)},
      {Point(1, 1)},
      {Point(0, 0)},
      {Point(1, 1)},
      {Point(0, 1)},
  });
  auto& host_buffer = pass.GetTransientsBuffer();
  auto vtx_buffer = vtx_builder.CreateVertexBuffer(host_buffer);
  cmd.BindVertices(vtx_buffer);

  VS::FrameInfo frame_info;
  frame_info.mvp = Matrix::MakeOrthographic(ISize(1, 1));

  auto sampler = renderer.GetContext()->GetSamplerLibrary()->GetSampler({});
  FS::BindInputTexture(cmd, input_snapshot->texture, sampler);

  VS::BindFrameInfo(cmd, host_buffer.EmplaceUniform(frame_info));

  return pass.AddCommand(std::move(cmd));
}

}  // namespace impeller
