// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/save_layer_utils.h"

namespace impeller {

std::optional<Rect> ComputeSaveLayerCoverage(
    const Rect& content_coverage,
    const Matrix& effect_transform,
    const Rect& coverage_limit,
    const std::shared_ptr<FilterContents>& image_filter,
    bool flood_output_coverage,
    bool flood_input_coverage) {
  Rect coverage = content_coverage;
  // There are two conditions that currently trigger flood_input_coverage, the
  // first is the presence of a backdrop filter and the second is the presence
  // of a color filter that effects transparent black.
  //
  // Backdrop filters apply before the saveLayer is restored. The presence of
  // a backdrop filter causes the content coverage of the saveLayer to be
  // unbounded.
  //
  // If there is a color filter that needs to flood its output. The color filter
  // is applied before any image filters, so this floods input coverage and not
  // the output coverage. Technically, we only need to flood the output of the
  // color filter and could allocate a render target sized just to the content,
  // but we don't currenty have the means to do so. Flooding the coverage is a
  // non-optimal but technically correct way to render this.
  if (flood_input_coverage) {
    coverage = Rect::MakeMaximum();
  }

  // If flood_output_coverage is true, then the restore is applied with a
  // destructive blend mode that requires flooding to the coverage limit.
  // Technically we could only allocated a render target as big as the input
  // coverage and then use a decal sampling mode to perform the flood. Returning
  // the coverage limit is a correct but non optimal means of ensuring correct
  // rendering.
  if (flood_output_coverage) {
    return coverage_limit;
  }

  // The content coverage must be scaled by any image filters present on the
  // saveLayer paint. For example, if a saveLayer has a coverage limit of
  // 100x100, but it has a Matrix image filter that scales by one half, the
  // actual coverage limit is 200x200.
  if (image_filter) {
    std::optional<Rect> source_coverage_limit =
        image_filter->GetSourceCoverage(effect_transform, coverage_limit);
    if (!source_coverage_limit.has_value()) {
      // No intersection with parent coverage limit.
      return std::nullopt;
    }

    // Transform the input coverage into the global coordinate space before
    // computing the bounds limit intersection.
    return coverage.TransformBounds(effect_transform)
        .Intersection(source_coverage_limit.value());
  }

  // If the input coverage is maximum, just return the coverage limit that
  // is already in the global coordinate space.
  if (coverage.IsMaximum()) {
    return coverage_limit;
  }

  // Transform the input coverage into the global coordinate space before
  // computing the bounds limit intersection.
  return coverage.TransformBounds(effect_transform)
      .Intersection(coverage_limit);
}

}  // namespace impeller
