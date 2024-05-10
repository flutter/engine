// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/dl_golden_unittests.h"

#include "display_list/dl_blend_mode.h"
#include "display_list/dl_color.h"
#include "display_list/dl_paint.h"
#include "display_list/effects/dl_color_filter.h"
#include "flutter/display_list/dl_builder.h"
#include "gtest/gtest.h"
#include "impeller/geometry/constants.h"

namespace flutter {
namespace testing {

using impeller::PlaygroundBackend;
using impeller::PlaygroundTest;

using AiksTest = DlGoldenTest;

INSTANTIATE_PLAYGROUND_SUITE(AiksTest);

TEST_P(AiksTest, CanDrawPaint) {
  auto draw = [](DlCanvas* canvas,
                 const std::vector<std::unique_ptr<DlImage>>& images) {
    canvas->Scale(0.2, 0.2);
    DlPaint paint;
    paint.setColor(DlColor::kCyan());
    canvas->DrawPaint(paint);
  };

  DisplayListBuilder builder;
  draw(&builder, /*images=*/{});

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderImage) {
  auto draw = [](DlCanvas* canvas, const std::vector<sk_sp<DlImage>>& images) {
    FML_CHECK(images.size() >= 1);
    DlPaint paint;
    paint.setColor(DlColor::kRed());
    canvas->DrawImage(images[0], SkPoint::Make(100.0, 100.0),
                      DlImageSampling::kLinear, &paint);
  };

  DisplayListBuilder builder;
  std::vector<sk_sp<DlImage>> images;
  images.emplace_back(CreateDlImageForFixture("kalimba.jpg"));
  draw(&builder, images);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, RotateColorFilteredPath) {
  DisplayListBuilder builder;
  builder.Transform(SkMatrix::Translate(300, 300));
  builder.Transform(SkMatrix::RotateDeg(impeller::kPiOver2));

  SkPath arrow_stem;
  SkPath arrow_head;

  arrow_stem.moveTo({120, 190}).lineTo({120, 50});
  arrow_head.moveTo({50, 120}).lineTo({120, 190}).lineTo({190, 120});

  auto filter =
      DlBlendColorFilter::Make(DlColor::kAliceBlue(), DlBlendMode::kSrcIn);

  DlPaint paint;
  paint.setStrokeMiter(15.0);
  paint.setStrokeCap(DlStrokeCap::kRound);
  paint.setStrokeJoin(DlStrokeJoin::kRound);
  paint.setDrawStyle(DlDrawStyle::kStroke);
  paint.setColorFilter(filter);

  builder.DrawPath(arrow_stem, paint);
  builder.DrawPath(arrow_head, paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

}  // namespace testing
}  // namespace flutter
