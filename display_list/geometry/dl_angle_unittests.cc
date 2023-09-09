// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_angle.h"
#include "fml/logging.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlAngleTest, DefaultConstructor) {
  DlAngle angle;
  EXPECT_EQ(angle.radians(), 0.0f);
  EXPECT_EQ(angle.degrees(), 0.0f);
}

TEST(DlAngleTest, RadiansEmptyConstructor) {
  DlAngle angle = DlAngle::Radians();
  EXPECT_EQ(angle.radians(), 0.0f);
  EXPECT_EQ(angle.degrees(), 0.0f);
}

TEST(DlAngleTest, DegreesEmptyConstructor) {
  DlAngle angle = DlAngle::Degrees();
  EXPECT_EQ(angle.radians(), 0.0f);
  EXPECT_EQ(angle.degrees(), 0.0f);
}

TEST(DlAngleTest, RadiansSimpleConstructor) {
  DlAngle angle = DlAngle::Radians(kDlScalar_Pi);
  EXPECT_EQ(angle.radians(), kDlScalar_Pi);
  EXPECT_EQ(angle.degrees(), 180.0f);
}

TEST(DlAngleTest, DegreesSimpleConstructor) {
  DlAngle angle = DlAngle::Degrees(180.0f);
  EXPECT_EQ(angle.radians(), kDlScalar_Pi);
  EXPECT_EQ(angle.degrees(), 180.0f);
}

TEST(DlAngleTest, AssignmentFromRadians) {
  DlAngle angle = DlAngle::Radians(kDlScalar_Pi);
  EXPECT_EQ(angle.radians(), kDlScalar_Pi);
  EXPECT_EQ(angle.degrees(), 180.0f);
  DlAngle copy = angle;
  EXPECT_EQ(copy.radians(), kDlScalar_Pi);
  EXPECT_EQ(copy.degrees(), 180.0f);
}

TEST(DlAngleTest, AssignmentFromDegrees) {
  DlAngle angle = DlAngle::Degrees(180.0f);
  EXPECT_EQ(angle.radians(), kDlScalar_Pi);
  EXPECT_EQ(angle.degrees(), 180.0f);
  DlAngle copy = angle;
  EXPECT_EQ(copy.radians(), kDlScalar_Pi);
  EXPECT_EQ(copy.degrees(), 180.0f);
}

TEST(DlAngleTest, CosSin) {
  for (int i = -360; i <= 720; i++) {
    DlScalar radians = i * kDlScalar_Pi / 180.0f;
    {
      DlFVector cos_sin = DlAngle::Degrees(i).CosSin();
      EXPECT_TRUE(DlScalar_IsNearlyZero(cos_sin.x() - cosf(radians)));
      EXPECT_TRUE(DlScalar_IsNearlyZero(cos_sin.y() - sinf(radians)));
    }
    {
      DlFVector cos_sin = DlAngle::Radians(radians).CosSin();
      EXPECT_TRUE(DlScalar_IsNearlyZero(cos_sin.x() - cosf(radians)));
      EXPECT_TRUE(DlScalar_IsNearlyZero(cos_sin.y() - sinf(radians)));
    }
  }
}

TEST(DlAngleTest, Quadrants) {
  auto test_quadrant = [](DlAngle angle, int quadrant) {
    DlFVector expect_cos_sin;
    switch (quadrant & 0x3) {
      case 0:
        EXPECT_TRUE(angle.IsFullCircle()) << angle << ", " << quadrant;
        expect_cos_sin = DlFVector(1.0f, 0.0f);
        break;

      case 1:
        EXPECT_FALSE(angle.IsFullCircle()) << angle << ", " << quadrant;
        expect_cos_sin = DlFVector(0.0f, 1.0f);
        break;

      case 2:
        EXPECT_FALSE(angle.IsFullCircle()) << angle << ", " << quadrant;
        expect_cos_sin = DlFVector(-1.0f, 0.0f);
        break;

      case 3:
        EXPECT_FALSE(angle.IsFullCircle()) << angle << ", " << quadrant;
        expect_cos_sin = DlFVector(0.0f, -1.0f);
        break;

      default:
        FML_DCHECK((quadrant & 3) >= 0 && (quadrant & 3) < 4);
    }
    EXPECT_EQ(angle.CosSin(), expect_cos_sin) << angle << ", " << quadrant;
  };

  // After about 150 full loops the math starts becoming imprecise enough
  // to affect the tests and the trig values.
  for (int quadrant = -600; quadrant <= 600; quadrant++) {
    test_quadrant(DlAngle::Degrees(quadrant * 90), quadrant);
    test_quadrant(DlAngle::Radians(quadrant * kDlScalar_Pi * 0.5f), quadrant);
  }
}

TEST(DlAngleTest, FullCircle) {
  auto test_full_circle = [](DlAngle angle) {
    EXPECT_TRUE(angle.IsFullCircle()) << angle;
    DlFVector cos_sin = angle.CosSin();
    EXPECT_EQ(cos_sin.x(), 1.0f) << angle;
    EXPECT_EQ(cos_sin.y(), 0.0f) << angle;
  };

  // After about 150 full loops the math starts becoming imprecise enough
  // to affect the tests and the trig values.
  for (int loops = -150; loops <= 150; loops++) {
    test_full_circle(DlAngle::Degrees(loops * 360));
    test_full_circle(DlAngle::Radians(loops * kDlScalar_Pi * 2.0f));
  }
}

TEST(DlAngleTest, NaNInfinityCosSin) {
  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  DlScalar pos_inf = std::numeric_limits<DlScalar>::infinity();
  DlScalar neg_inf = -std::numeric_limits<DlScalar>::infinity();

  ASSERT_EQ(DlAngle::Degrees(nan).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlAngle::Degrees(pos_inf).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlAngle::Degrees(neg_inf).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlAngle::Radians(nan).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlAngle::Radians(pos_inf).CosSin(), DlFVector(1.0f, 0.0f));
  ASSERT_EQ(DlAngle::Radians(neg_inf).CosSin(), DlFVector(1.0f, 0.0f));

  ASSERT_EQ(DlAngle::Degrees(nan).CosSin(), DlAngle::Degrees(0).CosSin());
  ASSERT_EQ(DlAngle::Degrees(pos_inf).CosSin(), DlAngle::Degrees(0).CosSin());
  ASSERT_EQ(DlAngle::Degrees(neg_inf).CosSin(), DlAngle::Degrees(0).CosSin());
  ASSERT_EQ(DlAngle::Radians(nan).CosSin(), DlAngle::Radians(0).CosSin());
  ASSERT_EQ(DlAngle::Radians(pos_inf).CosSin(), DlAngle::Radians(0).CosSin());
  ASSERT_EQ(DlAngle::Radians(neg_inf).CosSin(), DlAngle::Radians(0).CosSin());
}

}  // namespace testing
}  // namespace flutter
