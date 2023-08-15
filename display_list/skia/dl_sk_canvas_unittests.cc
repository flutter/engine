// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display_list/dl_tile_mode.h"
#include "display_list/effects/dl_color_source.h"
#include "flutter/display_list/dl_paint.h"
#include "flutter/display_list/skia/dl_sk_canvas.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DisplayListSkCanvas, ToSkDitheringDisabled) {
  // Test that when using the utility method "ToSk", the resulting SkPaint
  // does not have "isDither" set to true, even if it's requested by the
  // Flutter (dart:ui) paint, because it's not a supported feature in the
  // Impeller backend.

  // Create a new DlPaint with isDither set to true.
  //
  // This mimics the behavior of ui.Paint.enableDithering = true.
  DlPaint dl_paint;
  dl_paint.setDither(true);

  SkPaint sk_paint = ToSk(dl_paint);

  EXPECT_FALSE(sk_paint.isDither());
}

TEST(DisplayListSkCanvas, ToSkDitheringEnabledForGradients) {
  // Test that when using the utility method "ToSk", the resulting SkPaint
  // has "isDither" set to true, if the paint is a gradient, because it's
  // a supported feature in the Impeller backend.

  // Create a new DlPaint with isDither set to true.
  //
  // This mimics the behavior of ui.Paint.enableDithering = true.
  DlPaint dl_paint;
  dl_paint.setDither(true);

  // Set the paint to be a gradient.
  dl_paint.setColorSource(DlColorSource::MakeLinear(SkPoint::Make(0, 0),
                                                    SkPoint::Make(100, 100), 0,
                                                    0, 0, DlTileMode::kClamp));

  SkPaint sk_paint = ToSk(dl_paint);

  EXPECT_TRUE(sk_paint.isDither());
}

}  // namespace testing
}  // namespace flutter
