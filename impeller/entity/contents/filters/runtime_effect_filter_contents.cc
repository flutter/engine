// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/runtime_effect_filter_contents.h"

#include <cstring>
#include <optional>

#include "impeller/base/validation.h"
#include "impeller/entity/contents/anonymous_contents.h"
#include "impeller/entity/contents/runtime_effect_contents.h"
#include "impeller/geometry/size.h"

namespace impeller {

void RuntimeEffectFilterContents::SetRuntimeStage(
    std::shared_ptr<RuntimeStage> runtime_stage) {
  runtime_stage_ = std::move(runtime_stage);
}

void RuntimeEffectFilterContents::SetUniforms(
    std::shared_ptr<std::vector<uint8_t>> uniforms) {
  uniforms_ = std::move(uniforms);
}

void RuntimeEffectFilterContents::SetTextureInputs(
    std::vector<RuntimeEffectContents::TextureInput> texture_inputs) {
  texture_inputs_ = std::move(texture_inputs);
}

void RuntimeEffectFilterContents::SetMatrix(const Matrix& matrix) {
  matrix_ = matrix;
}

// |FilterContents|
std::optional<Entity> RuntimeEffectFilterContents::RenderFilter(
    const FilterInput::Vector& inputs,
    const ContentContext& renderer,
    const Entity& entity,
    const Matrix& effect_transform,
    const Rect& coverage,
    const std::optional<Rect>& coverage_hint) const {
  if (inputs.empty()) {
    return std::nullopt;
  }

  auto snapshot = inputs[0]->GetSnapshot("Matrix", renderer, entity);
  if (!snapshot.has_value()) {
    return std::nullopt;
  }

  if (rendering_mode_ ==
          Entity::RenderingMode::kSubpassPrependSnapshotTransform ||
      rendering_mode_ ==
          Entity::RenderingMode::kSubpassAppendSnapshotTransform) {
    // See Note in MatrixFilterContents::RenderFilter
    snapshot->transform = CalculateSubpassTransform(
        snapshot->transform, effect_transform, matrix_, rendering_mode_);
  } else {
    snapshot->transform = entity.GetTransform() *           //
                          matrix_ *                         //
                          entity.GetTransform().Invert() *  //
                          snapshot->transform;
  }

  std::optional<Rect> maybe_input_coverage = snapshot->GetCoverage();
  if (!maybe_input_coverage.has_value()) {
    return std::nullopt;
  }
  Rect input_coverage = maybe_input_coverage.value();
  // The shader is required to have at least one sampler, the first of
  // which is treated as the input and a vec2 size uniform to compute the
  // offsets. These are validated at the dart:ui layer, but to avoid crashes we
  // check here too.
  if (texture_inputs_.size() < 1 || uniforms_->size() < 8) {
    VALIDATION_LOG
        << "Invalid fragment shader in RuntimeEffectFilterContents. "
        << "Shader must have at least one sampler and a vec2 size uniform.";
    return std::nullopt;
  }

  // Update uniform values.
  std::vector<RuntimeEffectContents::TextureInput> texture_input_copy =
      texture_inputs_;
  texture_input_copy[0].texture = snapshot->texture;

  Size size = Size(snapshot->texture->GetSize());
  FML_LOG(ERROR) << "Size: " << size;
  memcpy(uniforms_->data(), &size, sizeof(Size));

  //----------------------------------------------------------------------------
  /// Create AnonymousContents for rendering.
  ///
  RenderProc render_proc =
      [snapshot, runtime_stage = runtime_stage_, uniforms = uniforms_,
       texture_inputs = texture_input_copy,
       input_coverage](const ContentContext& renderer, const Entity& entity,
                       RenderPass& pass) -> bool {
    RuntimeEffectContents contents;
    RectGeometry geom(Rect::MakeSize(input_coverage.GetSize()));
    contents.SetRuntimeStage(runtime_stage);
    contents.SetUniformData(uniforms);
    contents.SetTextureInputs(texture_inputs);
    contents.SetGeometry(&geom);
    return contents.Render(renderer, entity, pass);
  };

  CoverageProc coverage_proc =
      [coverage](const Entity& entity) -> std::optional<Rect> {
    return coverage.TransformBounds(entity.GetTransform());
  };

  auto contents = AnonymousContents::Make(render_proc, coverage_proc);

  Entity sub_entity;
  sub_entity.SetContents(std::move(contents));
  sub_entity.SetBlendMode(entity.GetBlendMode());
  sub_entity.SetTransform(snapshot->transform);
  return sub_entity;
}

void RuntimeEffectFilterContents::SetRenderingMode(
    Entity::RenderingMode rendering_mode) {
  rendering_mode_ = rendering_mode;
}

std::optional<Rect> RuntimeEffectFilterContents::GetFilterCoverage(
    const FilterInput::Vector& inputs,
    const Entity& entity,
    const Matrix& effect_transform) const {
  if (inputs.empty()) {
    return std::nullopt;
  }

  std::optional<Rect> coverage = inputs[0]->GetCoverage(entity);
  if (!coverage.has_value()) {
    return std::nullopt;
  }

  Matrix input_transform = inputs[0]->GetTransform(entity);
  if (rendering_mode_ ==
          Entity::RenderingMode::kSubpassPrependSnapshotTransform ||
      rendering_mode_ ==
          Entity::RenderingMode::kSubpassAppendSnapshotTransform) {
    Rect coverage_bounds = coverage->TransformBounds(input_transform.Invert());
    Matrix transform = CalculateSubpassTransform(
        input_transform, effect_transform, matrix_, rendering_mode_);
    return coverage_bounds.TransformBounds(transform);
  } else {
    Matrix transform = input_transform *          //
                       matrix_ *                  //
                       input_transform.Invert();  //
    return coverage->TransformBounds(transform);
  }
}

// |FilterContents|
std::optional<Rect> RuntimeEffectFilterContents::GetFilterSourceCoverage(
    const Matrix& effect_transform,
    const Rect& output_limit) const {
  auto transform = effect_transform *          //
                   matrix_ *                   //
                   effect_transform.Invert();  //
  if (transform.GetDeterminant() == 0.0) {
    return std::nullopt;
  }
  auto inverse = transform.Invert();
  return output_limit.TransformBounds(inverse);
}

}  // namespace impeller
