// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/dl_golden_unittests.h"

#include "flutter/display_list/dl_builder.h"
#include "flutter/testing/testing.h"
#include "gtest/gtest.h"
#include "impeller/display_list/dl_dispatcher.h"

namespace impeller {
namespace testing {

INSTANTIATE_PLAYGROUND_SUITE(DlGoldenTest);

TEST_P(DlGoldenTest, CanDrawPaint) {
  auto draw = [](flutter::DlCanvas* canvas, flutter::DlImage** images) {
    canvas->Scale(0.2, 0.2);
    flutter::DlPaint paint;
    paint.setColor(flutter::DlColor::kCyan());
    canvas->DrawPaint(paint);
  };

  flutter::DisplayListBuilder builder;
  draw(&builder, nullptr);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

}  // namespace testing
}  // namespace impeller
