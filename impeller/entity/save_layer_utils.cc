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
    bool destructive_blend,
    bool has_backdrop_filter,
    bool bounds_from_caller) {
  // If the saveLayer is unbounded, the coverage is the same as the
  // coverage limit. By default, the coverage limit begins as the screen
  // coverage. Either a lack of bounds, or the presence of a backdrop
  // effecting paint property indicates that the saveLayer is unbounded.
  // The o  ne special case is when the input coverage was specified by the
  // developer, in this case we still use the content coverage even if the
  // saveLayer itself is unbounded.
  bool save_layer_is_unbounded = has_backdrop_filter || destructive_blend;

  // Otherwise, the save layer is bounded by either its contents or by
  // a specified coverage limit. In these cases the coverage value is used
  // and intersected with the coverage limit.
  Rect input_coverage = (save_layer_is_unbounded && !bounds_from_caller)
                            ? Rect::MakeMaximum()
                            : content_coverage;

  // The content coverage must be scaled by any image filters present on the
  // saveLayer paint. For example, if a saveLayer has a coverage limit of
  // 100x100, but it has a Matrix image filter that scales by one half, the
  // actual coverage limit is 200x200.
  //
  // If there is no image filter, then we can assume that the contents have
  // absorbed the current transform and thus it has already been incorporated
  // into any computed bounds. That is, a canvas scaling transform of 0.5
  // changes the coverage of the contained entities directly and doesnt need to
  // be additionally incorporated into the coverage computation here.
  std::optional<Rect> result_bounds;
  if (image_filter) {
    std::optional<Rect> source_coverage_limit =
        image_filter->GetSourceCoverage(effect_transform, coverage_limit);
    if (!source_coverage_limit.has_value()) {
      // No intersection with parent coverage limit.
      return std::nullopt;
    }
    result_bounds = input_coverage.Intersection(source_coverage_limit.value());
  } else {
    result_bounds = input_coverage.Intersection(coverage_limit);
  }

  // Finally, transform the resulting bounds back into the global coordinate
  // space.
  if (result_bounds.has_value()) {
    return result_bounds->TransformBounds(effect_transform);
  }
  return std::nullopt;
}

}  // namespace impeller
