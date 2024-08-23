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
    bool flood_clip) {
  // If the saveLayer is unbounded, the coverage is the same as the
  // coverage limit. By default, the coverage limit begins as the screen
  // coverage. Either a lack of bounds, or the presence of a backdrop
  // effecting paint property indicates that the saveLayer is unbounded.
  // Otherwise, the save layer is bounded by either its contents or by
  // a specified coverage limit. In these cases the coverage value is used
  // and intersected with the coverage limit.
  if (flood_clip) {
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
    return content_coverage.TransformBounds(effect_transform)
        .Intersection(source_coverage_limit.value());
  }

  // If the input coverage is maximum, just return the coverage limit that
  // is already in the global coordinate space.
  if (content_coverage.IsMaximum()) {
    return coverage_limit;
  }

  // Transform the input coverage into the global coordinate space before
  // computing the bounds limit intersection.
  return content_coverage.TransformBounds(effect_transform)
      .Intersection(coverage_limit);
}

}  // namespace impeller
