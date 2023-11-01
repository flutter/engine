// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/gaussian_blur_filter_contents.h"

namespace impeller {

GaussianBlurFilterContents::GaussianBlurFilterContents(Scalar sigma)
    : sigma_(sigma) {}

std::optional<Rect> GaussianBlurFilterContents::GetFilterSourceCoverage(
    const Matrix& effect_transform,
    const Rect& output_limit) const {
  return {};
}

std::optional<Rect> GaussianBlurFilterContents::GetFilterCoverage(
    const FilterInput::Vector& inputs,
    const Entity& entity,
    const Matrix& effect_transform) const {
  if (inputs.empty()) {
    return {};
  }

  std::optional<Rect> input_coverage = inputs[0]->GetCoverage(entity);
  if (!input_coverage.has_value()) {
    return {};
  }

  Scalar blur_radius = CalculateBlurRadius(sigma_);
  Vector3 blur_radii = effect_transform.Basis() * Vector3{blur_radius, blur_radius, 0.0};
  return Rect::MakeLTRB(input_coverage.value().GetLeft() - blur_radii.y,
                        input_coverage.value().GetTop() - blur_radii.x,
                        input_coverage.value().GetRight() + blur_radii.x,
                        input_coverage.value().GetBottom() + blur_radii.y);
}

std::optional<Entity> GaussianBlurFilterContents::RenderFilter(
    const FilterInput::Vector& input_textures,
    const ContentContext& renderer,
    const Entity& entity,
    const Matrix& effect_transform,
    const Rect& coverage,
    const std::optional<Rect>& coverage_hint) const {
  return {};
}

Scalar GaussianBlurFilterContents::CalculateBlurRadius(Scalar sigma) {
  return static_cast<Radius>(Sigma(sigma)).radius;
}

}  // namespace impeller
