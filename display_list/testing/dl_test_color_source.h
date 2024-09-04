// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_TESTING_DL_TEST_COLOR_SOURCE_H_
#define FLUTTER_DISPLAY_LIST_TESTING_DL_TEST_COLOR_SOURCE_H_

#include "flutter/display_list/effects/dl_color_source.h"

namespace flutter {
namespace testing {

std::shared_ptr<DlLinearGradientColorSource> MakeLinearColorSource(
    const SkPoint start_point,
    const SkPoint end_point,
    uint32_t stop_count,
    const DlColor* colors,
    const float* stops,
    DlTileMode tile_mode,
    const SkMatrix* matrix = nullptr);
}
}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_TESTING_DL_TEST_COLOR_SOURCE_H_
