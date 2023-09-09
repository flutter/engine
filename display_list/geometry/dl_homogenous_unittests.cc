// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_homogenous.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlHomogenousTest, EmptyConstructor) {
  DlFHomogenous3D hpt;
  EXPECT_EQ(hpt.x(), 0.0f);
  EXPECT_EQ(hpt.y(), 0.0f);
  EXPECT_EQ(hpt.z(), 0.0f);
  EXPECT_EQ(hpt.w(), 1.0f);
  ASSERT_TRUE(hpt.IsFinite());
  ASSERT_TRUE(hpt.IsUnclipped());
  EXPECT_EQ(hpt.Normalize(), DlFHomogenous3D());
  EXPECT_EQ(hpt.NormalizeToPoint(), DlFPoint());
}

TEST(DlHomogenousTest, DefaultConstructor) {
  DlFHomogenous3D hpt = DlFHomogenous3D();
  EXPECT_EQ(hpt.x(), 0.0f);
  EXPECT_EQ(hpt.y(), 0.0f);
  EXPECT_EQ(hpt.z(), 0.0f);
  EXPECT_EQ(hpt.w(), 1.0f);
  ASSERT_TRUE(hpt.IsFinite());
  ASSERT_TRUE(hpt.IsUnclipped());
  EXPECT_EQ(hpt.Normalize(), DlFHomogenous3D());
  EXPECT_EQ(hpt.NormalizeToPoint(), DlFPoint());
}

TEST(DlHomogenousTest, XYConstructor) {
  DlFHomogenous3D hpt = DlFHomogenous3D(2.0f, 3.0f);
  EXPECT_EQ(hpt.x(), 2.0f);
  EXPECT_EQ(hpt.y(), 3.0f);
  EXPECT_EQ(hpt.z(), 0.0f);
  EXPECT_EQ(hpt.w(), 1.0f);
  ASSERT_TRUE(hpt.IsFinite());
  ASSERT_TRUE(hpt.IsUnclipped());
  EXPECT_EQ(hpt.Normalize(), DlFHomogenous3D(2.0f, 3.0f, 0.0f, 1.0f));
  EXPECT_EQ(hpt.NormalizeToPoint(), DlFPoint(2.0f, 3.0f));
}

TEST(DlHomogenousTest, PointConstructor) {
  DlFHomogenous3D hpt = DlFHomogenous3D(DlFPoint(2.0f, 3.0f));
  EXPECT_EQ(hpt.x(), 2.0f);
  EXPECT_EQ(hpt.y(), 3.0f);
  EXPECT_EQ(hpt.z(), 0.0f);
  EXPECT_EQ(hpt.w(), 1.0f);
  ASSERT_TRUE(hpt.IsFinite());
  ASSERT_TRUE(hpt.IsUnclipped());
  EXPECT_EQ(hpt.Normalize(), DlFHomogenous3D(2.0f, 3.0f, 0.0f, 1.0f));
  EXPECT_EQ(hpt.NormalizeToPoint(), DlFPoint(2.0f, 3.0f));
}

TEST(DlHomogenousTest, XYZConstructor) {
  DlFHomogenous3D hpt = DlFHomogenous3D(2.0f, 3.0f, 4.0f);
  EXPECT_EQ(hpt.x(), 2.0f);
  EXPECT_EQ(hpt.y(), 3.0f);
  EXPECT_EQ(hpt.z(), 4.0f);
  EXPECT_EQ(hpt.w(), 1.0f);
  ASSERT_TRUE(hpt.IsFinite());
  ASSERT_TRUE(hpt.IsUnclipped());
  EXPECT_EQ(hpt.Normalize(), DlFHomogenous3D(2.0f, 3.0f, 4.0f, 1.0f));
  EXPECT_EQ(hpt.NormalizeToPoint(), DlFPoint(2.0f, 3.0f));
}

TEST(DlHomogenousTest, XYZWConstructor) {
  DlFHomogenous3D hpt = DlFHomogenous3D(2.0f, 4.0f, 6.0f, 8.0f);
  EXPECT_EQ(hpt.x(), 2.0f);
  EXPECT_EQ(hpt.y(), 4.0f);
  EXPECT_EQ(hpt.z(), 6.0f);
  EXPECT_EQ(hpt.w(), 8.0f);
  ASSERT_TRUE(hpt.IsFinite());
  ASSERT_TRUE(hpt.IsUnclipped());
  EXPECT_EQ(hpt.Normalize(), DlFHomogenous3D(0.25f, 0.5f, 0.75f, 1.0f));
  EXPECT_EQ(hpt.NormalizeToPoint(), DlFPoint(0.25f, 0.5f));
}

TEST(DlHomogenousTest, NaNInfinityCoordinates) {
  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  DlScalar pos_inf = std::numeric_limits<DlScalar>::infinity();
  DlScalar neg_inf = -std::numeric_limits<DlScalar>::infinity();

  DlFHomogenous3D hpt = DlFHomogenous3D(2.0f, 4.0f, 6.0f, 8.0f);
  ASSERT_TRUE(hpt.IsFinite());
  ASSERT_TRUE(hpt.IsUnclipped());
  ASSERT_EQ(hpt.Normalize(), DlFHomogenous3D(0.25f, 0.5f, 0.75f, 1.0f));
  ASSERT_EQ(hpt.NormalizeToPoint(), DlFPoint(0.25f, 0.5f));

  auto test_non_finite = [](const DlFHomogenous3D& hpt, int mask,
                            DlScalar non_finite) {
    ASSERT_TRUE(hpt.IsFinite());
    std::string label = "with " + std::to_string(non_finite) +  //
                        " in " + std::to_string(mask);

    DlScalar x = (mask & (1 << 0)) ? non_finite : hpt.x();
    DlScalar y = (mask & (1 << 1)) ? non_finite : hpt.y();
    DlScalar z = (mask & (1 << 2)) ? non_finite : hpt.z();
    DlScalar w = (mask & (1 << 3)) ? non_finite : hpt.w();

    DlFHomogenous3D hpt_nonf = DlFHomogenous3D(x, y, z, w);
    if (mask == 0) {
      ASSERT_TRUE(hpt_nonf.IsFinite()) << label;
      ASSERT_EQ(hpt, hpt_nonf) << label;
    } else {
      ASSERT_FALSE(hpt_nonf.IsFinite()) << label;
    }

    DlFHomogenous3D hpt_nonf_norm = hpt_nonf.Normalize();
    DlFPoint hpt_nonf_pt = hpt_nonf.NormalizeToPoint();
    // Whether or not hpt_nan has a non-finite value in it, the normalized
    // results will be finite.
    ASSERT_TRUE(hpt_nonf_norm.IsFinite()) << label;
    ASSERT_TRUE(hpt_nonf_pt.IsFinite()) << label;
    ASSERT_EQ(hpt_nonf_norm.w(), 1.0f) << label;
    if (mask == 0) {
      ASSERT_EQ(hpt_nonf_norm, DlFHomogenous3D(0.25f, 0.5f, 0.75f, 1.0f))
          << label;
      ASSERT_EQ(hpt_nonf_pt, DlFPoint(0.25f, 0.5f)) << label;
    } else {
      ASSERT_EQ(hpt_nonf_norm, DlFHomogenous3D()) << label;
      ASSERT_EQ(hpt_nonf_pt, DlFPoint()) << label;
    }
  };

  for (int mask = 0; mask < 16; mask++) {
    test_non_finite(hpt, mask, nan);
    test_non_finite(hpt, mask, pos_inf);
    test_non_finite(hpt, mask, neg_inf);
  }
}

}  // namespace testing
}  // namespace flutter
