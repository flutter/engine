// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/dl_color.h"
#include "flutter/testing/testing.h"

#include "third_party/skia/include/core/SkColor.h"

namespace flutter {
namespace testing {

static void arraysEqual(const uint32_t* ints,
                        const DlColor* colors,
                        int count) {
  for (int i = 0; i < count; i++) {
    EXPECT_TRUE(ints[i] == colors[i].argb());
  }
}

TEST(DisplayListColor, ArrayInterchangeableWithUint32) {
  uint32_t ints[5] = {
      0xFF000000,  //
      0xFFFF0000,  //
      0xFF00FF00,  //
      0xFF0000FF,  //
      0xF1F2F3F4,
  };
  DlColor colors[5] = {
      DlColor::kBlack(),  //
      DlColor::kRed(),    //
      DlColor::kGreen(),  //
      DlColor::kBlue(),   //
      DlColor(0xF1F2F3F4),
  };
  arraysEqual(ints, colors, 5);
  arraysEqual(reinterpret_cast<const uint32_t*>(colors),
              reinterpret_cast<const DlColor*>(ints), 5);
}

TEST(DisplayListColor, DlColorDirectlyComparesToSkColor) {
  EXPECT_EQ(DlColor::kBlack(), SK_ColorBLACK);
  EXPECT_EQ(DlColor::kRed(), SK_ColorRED);
  EXPECT_EQ(DlColor::kGreen(), SK_ColorGREEN);
  EXPECT_EQ(DlColor::kBlue(), SK_ColorBLUE);
}

TEST(DisplayListColor, ScalarFactory) {
  // Test 9 standard colors with their Scalar equivalents
  EXPECT_EQ(DlColor::MakeARGB(0.0f, 0.0f, 0.0f, 0.0f), DlColor::kTransparent());
  EXPECT_EQ(DlColor::MakeARGB(1.0f, 0.0f, 0.0f, 0.0f), DlColor::kBlack());
  EXPECT_EQ(DlColor::MakeARGB(1.0f, 1.0f, 1.0f, 1.0f), DlColor::kWhite());

  EXPECT_EQ(DlColor::MakeARGB(1.0f, 1.0f, 0.0f, 0.0f), DlColor::kRed());
  EXPECT_EQ(DlColor::MakeARGB(1.0f, 0.0f, 1.0f, 0.0f), DlColor::kGreen());
  EXPECT_EQ(DlColor::MakeARGB(1.0f, 0.0f, 0.0f, 1.0f), DlColor::kBlue());

  EXPECT_EQ(DlColor::MakeARGB(1.0f, 0.0f, 1.0f, 1.0f), DlColor::kCyan());
  EXPECT_EQ(DlColor::MakeARGB(1.0f, 1.0f, 0.0f, 1.0f), DlColor::kMagenta());
  EXPECT_EQ(DlColor::MakeARGB(1.0f, 1.0f, 1.0f, 0.0f), DlColor::kYellow());

  // Test each component reduced to half intensity
  EXPECT_EQ(DlColor::MakeARGB(0.5f, 1.0f, 1.0f, 1.0f),
            DlColor::kWhite().withAlpha(0x80));
  EXPECT_EQ(DlColor::MakeARGB(1.0f, 0.5f, 1.0f, 1.0f),
            DlColor::kWhite().withRed(0x80));
  EXPECT_EQ(DlColor::MakeARGB(1.0f, 1.0f, 0.5f, 1.0f),
            DlColor::kWhite().withGreen(0x80));
  EXPECT_EQ(DlColor::MakeARGB(1.0f, 1.0f, 1.0f, 0.5f),
            DlColor::kWhite().withBlue(0x80));

  // Test clamping to [0.0, 1.0]
  EXPECT_EQ(DlColor::MakeARGB(-1.0f, -1.0f, -1.0f, -1.0f),
            DlColor::kTransparent());
  EXPECT_EQ(DlColor::MakeARGB(2.0f, 2.0f, 2.0f, 2.0f), DlColor::kWhite());
}

}  // namespace testing
}  // namespace flutter
