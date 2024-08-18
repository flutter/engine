// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_AIKS_SAVE_LAYER_UTILS_H_
#define FLUTTER_IMPELLER_AIKS_SAVE_LAYER_UTILS_H_

#include <memory>
#include <optional>

#include "impeller/aiks/image_filter.h"
#include "impeller/aiks/paint.h"

namespace impeller {

/// Compute the coverage of the subpass in the subpass coordinate space.
///
/// A return value of std::nullopt indicates that the coverage is empty or
/// otherwise does not intersect with the parent coverage limit and should
/// be discarded.
std::optional<Rect> ComputeSaveLayerCoverage(
    Rect content_coverage,
    const std::shared_ptr<ImageFilter>& backdrop_filter,
    const Paint& paint,
    const Matrix& effect_transform,
    const Rect& coverage_limit,
    bool user_provided_bounds = false);

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_AIKS_SAVE_LAYER_UTILS_H_
