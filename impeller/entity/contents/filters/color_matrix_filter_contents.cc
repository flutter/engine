// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/color_matrix_filter_contents.h"

#include <optional>

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/geometry/vector.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"

namespace impeller {

ColorMatrixFilterContents::ColorMatrixFilterContents() = default;

ColorMatrixFilterContents::~ColorMatrixFilterContents() = default;

void ColorMatrixFilterContents::SetMatrix(const ColorMatrix& matrix) {
  matrix_ = matrix;
}

std::optional<Rect> ColorMatrixFilterContents::GetFilterCoverage(
    const FilterInput::Vector& inputs,
    const Entity& entity) const {
  // TODO (kaushikiska@)
  return std::nullopt;
}

bool ColorMatrixFilterContents::RenderFilter(const FilterInput::Vector& inputs,
                                             const ContentContext& renderer,
                                             const Entity& entity,
                                             RenderPass& pass,
                                             const Rect& coverage) const {
  if (inputs.empty()) {
    return true;
  }

  using VS = ColorMatrixColorFilterPipeline::VertexShader;
  using FS = ColorMatrixColorFilterPipeline::FragmentShader;

  auto& host_buffer = pass.GetTransientsBuffer();

  auto input_snapshot = inputs[0]->GetSnapshot(renderer, entity);
  if (!input_snapshot.has_value()) {
    return true;
  }

  auto maybe_input_uvs = input_snapshot->GetCoverageUVs(coverage);
  if (!maybe_input_uvs.has_value()) {
    return true;
  }

  auto input_uvs = maybe_input_uvs.value();

  VertexBufferBuilder<VS::PerVertexData> vtx_builder;
  vtx_builder.AddVertices({
      {Point(0, 0), input_uvs[0]},
      {Point(1, 0), input_uvs[1]},
      {Point(1, 1), input_uvs[3]},
      {Point(0, 0), input_uvs[0]},
      {Point(1, 1), input_uvs[3]},
      {Point(0, 1), input_uvs[2]},
  });
  auto vtx_buffer = vtx_builder.CreateVertexBuffer(host_buffer);

  Command cmd;
  cmd.label = "Color Matrix Filter";
  auto options = OptionsFromPass(pass);
  options.blend_mode = Entity::BlendMode::kSource;

  cmd.pipeline = renderer.GetColorMatrixColorFilterPipeline(options);
  cmd.BindVertices(vtx_buffer);

  VS::FrameInfo frame_info;
  frame_info.mvp = Matrix::MakeOrthographic(ISize(1, 1));

  // clang-format off
  const float* matrix = matrix_.array;
  frame_info.m =
      Matrix(
        matrix[ 0], matrix[ 1], matrix[ 2], matrix[ 3],
        matrix[ 5], matrix[ 6], matrix[ 7], matrix[ 8],
        matrix[10], matrix[11], matrix[12], matrix[13],
        matrix[15], matrix[16], matrix[17], matrix[18]
      );
  frame_info.v = Vector4(matrix[4], matrix[9], matrix[14], matrix[19]);
  // clang-format on

  auto uniform_view = host_buffer.EmplaceUniform(frame_info);
  VS::BindFrameInfo(cmd, uniform_view);

  return pass.AddCommand(std::move(cmd));
}

}  // namespace impeller
