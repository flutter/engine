// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/matrix_filter_contents.h"

namespace impeller {

MatrixFilterContents::MatrixFilterContents() = default;

MatrixFilterContents::~MatrixFilterContents() = default;

void MatrixFilterContents::SetMatrix(Matrix matrix) {
  matrix_ = matrix;
}

std::optional<Snapshot> MatrixFilterContents::RenderFilter(
    const FilterInput::Vector& inputs,
    const ContentContext& renderer,
    const Entity& entity,
    const Matrix& effect_transform,
    const Rect& coverage) const {
  auto input_snapshot = inputs[0]->GetSnapshot(renderer, entity);
  if (!input_snapshot.has_value()) {
    return std::nullopt;
  }
  auto local_transform = inputs[0]->GetLocalTransform(entity);
  input_snapshot->transform = input_snapshot->transform *
                              local_transform.Invert() * matrix_ *
                              local_transform;
  return input_snapshot;
}

std::optional<Rect> MatrixFilterContents::GetFilterCoverage(
    const FilterInput::Vector& inputs,
    const Entity& entity,
    const Matrix& effect_transform) const {
  if (inputs.empty()) {
    return std::nullopt;
  }

  std::optional<Rect> input_coverage = inputs[0]->GetLocalCoverage(entity);
  if (!input_coverage.has_value()) {
    return std::nullopt;
  }
  return input_coverage->TransformBounds(entity.GetTransformation() * matrix_);
}

}  // namespace impeller
