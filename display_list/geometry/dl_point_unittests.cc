// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_point.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlPointTest, FPointEmptyConstructor) {
  DlFPoint pt;
  ASSERT_EQ(pt.x(), 0.0f);
  ASSERT_EQ(pt.y(), 0.0f);
  ASSERT_EQ(pt.Length(), 0.0f);
  ASSERT_TRUE(pt.IsFinite());
}

TEST(DlPointTest, IPointEmptyConstructor) {
  DlIPoint pt;
  ASSERT_EQ(pt.x(), 0);
  ASSERT_EQ(pt.y(), 0);
  ASSERT_EQ(pt.Length(), 0);
  // ASSERT_TRUE(pt.IsFinite()); // should fail to compile
}

TEST(DlPointTest, FPointDefaultConstructor) {
  DlFPoint pt = DlFPoint();
  ASSERT_EQ(pt.x(), 0.0f);
  ASSERT_EQ(pt.y(), 0.0f);
  ASSERT_EQ(pt.Length(), 0.0f);
  ASSERT_TRUE(pt.IsFinite());
}

TEST(DlPointTest, IPointDefaultConstructor) {
  DlIPoint pt = DlIPoint();
  ASSERT_EQ(pt.x(), 0);
  ASSERT_EQ(pt.y(), 0);
  ASSERT_EQ(pt.Length(), 0);
  // ASSERT_TRUE(pt.IsFinite()); // should fail to compile
}

TEST(DlPointTest, FPointXYConstructor) {
  DlFPoint pt = DlFPoint(3.0f, 4.0f);
  ASSERT_EQ(pt.x(), 3.0f);
  ASSERT_EQ(pt.y(), 4.0f);
  ASSERT_EQ(pt.Length(), 5.0f);
  ASSERT_TRUE(pt.IsFinite());
}

TEST(DlPointTest, IPointXYConstructor) {
  DlIPoint pt = DlIPoint(3, 4);
  ASSERT_EQ(pt.x(), 3);
  ASSERT_EQ(pt.y(), 4);
  ASSERT_EQ(pt.Length(), 5);
  // ASSERT_TRUE(pt.IsFinite()); // should fail to compile
}

TEST(DlPointTest, FPointFPointConstructor) {
  DlFPoint src = DlFPoint(3.0f, 4.0f);
  DlFPoint pt = DlFPoint(src);
  ASSERT_EQ(pt.x(), 3.0f);
  ASSERT_EQ(pt.y(), 4.0f);
  ASSERT_EQ(pt.Length(), 5.0f);
  ASSERT_TRUE(pt.IsFinite());
}

TEST(DlPointTest, IPointIPointConstructor) {
  DlIPoint src = DlIPoint(3, 4);
  DlIPoint pt = DlIPoint(src);
  ASSERT_EQ(pt.x(), 3);
  ASSERT_EQ(pt.y(), 4);
  ASSERT_EQ(pt.Length(), 5);
  // ASSERT_TRUE(pt.IsFinite()); // should fail to compile
}

TEST(DlPointTest, NaNInfinityCoordinates) {
  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  DlScalar pos_inf = std::numeric_limits<DlScalar>::infinity();
  DlScalar neg_inf = -std::numeric_limits<DlScalar>::infinity();

  DlFPoint pt = DlFPoint(2.0f, 4.0f);
  ASSERT_TRUE(pt.IsFinite());

  auto test_non_finite = [](const DlFPoint& pt, int mask,
                            DlScalar non_finite) {
    ASSERT_TRUE(pt.IsFinite());
    std::string label = "with " + std::to_string(non_finite) +  //
                        " in " + std::to_string(mask);

    DlScalar x = (mask & (1 << 0)) ? non_finite : pt.x();
    DlScalar y = (mask & (1 << 1)) ? non_finite : pt.y();

    DlFPoint pt_nonf = DlFPoint(x, y);
    if (mask == 0) {
      ASSERT_TRUE(pt_nonf.IsFinite()) << label;
      ASSERT_EQ(pt, pt_nonf) << label;
    } else {
      ASSERT_FALSE(pt_nonf.IsFinite()) << label;
      ASSERT_NE(pt, pt_nonf) << label;
    }
  };

  for (int mask = 0; mask <= 0x3; mask++) {
    test_non_finite(pt, mask, nan);
    test_non_finite(pt, mask, pos_inf);
    test_non_finite(pt, mask, neg_inf);
  }
}

}  // namespace testing
}  // namespace flutter
