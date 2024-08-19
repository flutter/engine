// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_SAVE_LAYER_UTILS_H_
#define FLUTTER_IMPELLER_ENTITY_SAVE_LAYER_UTILS_H_

#include <memory>
#include <optional>

#include "impeller/entity/contents/filters/filter_contents.h"
#include "impeller/geometry/rect.h"

namespace impeller {

/// Compute the coverage of the subpass in the subpass coordinate space.
///
/// A return value of std::nullopt indicates that the coverage is empty or
/// otherwise does not intersect with the parent coverage limit and should
/// be discarded.
std::optional<Rect> ComputeSaveLayerCoverage(
    const Rect& content_coverage,
    const Matrix& effect_transform,
    const Rect& coverage_limit,
    const std::shared_ptr<FilterContents>& image_filter,
    bool destructive_blend = false,
    bool has_backdrop_filter = false,
    bool bounds_from_caller = false);

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_SAVE_LAYER_UTILS_H_
