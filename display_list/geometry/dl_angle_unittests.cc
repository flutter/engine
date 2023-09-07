// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_angle.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlAngleTest, RadiansEmptyConstructor) {
  DlRadians radians;
  EXPECT_EQ(radians.radians(), 0.0);
  EXPECT_EQ(radians.degrees(), 0.0);
}

TEST(DlAngleTest, DegreesEmptyConstructor) {
  DlDegrees degrees;
  EXPECT_EQ(degrees.radians(), 0.0);
  EXPECT_EQ(degrees.degrees(), 0.0);
}

TEST(DlAngleTest, RadiansSimpleConstructor) {
  DlRadians radians(kDlScalar_Pi);
  EXPECT_EQ(radians.radians(), kDlScalar_Pi);
  EXPECT_EQ(radians.degrees(), 180.0);
}

TEST(DlAngleTest, DegreesSimpleConstructor) {
  DlDegrees degrees(180.0);
  EXPECT_EQ(degrees.radians(), kDlScalar_Pi);
  EXPECT_EQ(degrees.degrees(), 180.0);
}

TEST(DlAngleTest, RadiansToDegreesConversion) {
  DlRadians radians(kDlScalar_Pi);
  EXPECT_EQ(radians.radians(), kDlScalar_Pi);
  EXPECT_EQ(radians.degrees(), 180.0);
  DlDegrees degrees = radians;
  EXPECT_EQ(degrees.radians(), kDlScalar_Pi);
  EXPECT_EQ(degrees.degrees(), 180.0);
}

TEST(DlAngleTest, DegreesToRadiansConversion) {
  DlDegrees degrees(180.0);
  EXPECT_EQ(degrees.radians(), kDlScalar_Pi);
  EXPECT_EQ(degrees.degrees(), 180.0);
  DlRadians radians = degrees;
  EXPECT_EQ(radians.radians(), kDlScalar_Pi);
  EXPECT_EQ(radians.degrees(), 180.0);
}

TEST(DlAngleTest, RadiansToDegreesArgumentConversion) {
  DlRadians radians(kDlScalar_Pi);
  EXPECT_EQ(radians.radians(), kDlScalar_Pi);
  EXPECT_EQ(radians.degrees(), 180.0);
  auto test = [](const DlDegrees& degrees) {
    EXPECT_EQ(degrees.radians(), kDlScalar_Pi);
    EXPECT_EQ(degrees.degrees(), 180.0);
  };
  test(radians);
  auto test2 = [](const DlDegrees& degrees) {
    EXPECT_EQ(degrees.radians(), kDlScalar_Pi);
    EXPECT_EQ(degrees.degrees(), 180.0);
  };
  test2(radians);
}

TEST(DlAngleTest, DegreesToRadiansArgumentConversion) {
  DlDegrees degrees(180.0);
  EXPECT_EQ(degrees.radians(), kDlScalar_Pi);
  EXPECT_EQ(degrees.degrees(), 180.0);
  auto test = [](const DlRadians& radians) {
    EXPECT_EQ(radians.radians(), kDlScalar_Pi);
    EXPECT_EQ(radians.degrees(), 180.0);
  };
  test(degrees);
  auto test2 = [](const DlRadians& radians) {
    EXPECT_EQ(radians.radians(), kDlScalar_Pi);
    EXPECT_EQ(radians.degrees(), 180.0);
  };
  test2(degrees);
}

TEST(DlAngleTest, DegreesCosSin) {
  for (int i = -360; i <= 720; i++) {
    DlScalar radians = i * kDlScalar_Pi / 180.0;
    {
      DlFVector cos_sin = DlDegrees(i).CosSin();
      EXPECT_TRUE(DlScalar_IsNearlyZero(cos_sin.x() - cosf(radians)));
      EXPECT_TRUE(DlScalar_IsNearlyZero(cos_sin.y() - sinf(radians)));
    }
    {
      DlFVector cos_sin = DlRadians(radians).CosSin();
      EXPECT_TRUE(DlScalar_IsNearlyZero(cos_sin.x() - cosf(radians)));
      EXPECT_TRUE(DlScalar_IsNearlyZero(cos_sin.y() - sinf(radians)));
    }
  }
}

TEST(DlAngleTest, NaNInfinityCosSin) {
  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  DlScalar pos_inf = std::numeric_limits<DlScalar>::infinity();
  DlScalar neg_inf = -std::numeric_limits<DlScalar>::infinity();

  ASSERT_EQ(DlDegrees(nan).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlDegrees(pos_inf).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlDegrees(neg_inf).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlRadians(nan).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlRadians(pos_inf).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlRadians(neg_inf).CosSin(), DlFVector(1.0f, 0.0f));

  ASSERT_EQ(DlDegrees(nan).CosSin(), DlDegrees(0).CosSin());
  ASSERT_EQ(DlDegrees(pos_inf).CosSin(), DlDegrees(0).CosSin());
  ASSERT_EQ(DlDegrees(neg_inf).CosSin(), DlDegrees(0).CosSin());
  ASSERT_EQ(DlRadians(nan).CosSin(), DlRadians(0).CosSin());
  ASSERT_EQ(DlRadians(pos_inf).CosSin(), DlRadians(0).CosSin());
  ASSERT_EQ(DlRadians(neg_inf).CosSin(), DlRadians(0).CosSin());
}

}  // namespace testing
}  // namespace flutter
