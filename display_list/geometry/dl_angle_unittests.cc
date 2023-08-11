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
  DlRadians radians(M_PI);
  EXPECT_EQ(radians.radians(), kDlScalar_Pi);
  EXPECT_EQ(radians.degrees(), 180.0);
}

TEST(DlAngleTest, DegreesSimpleConstructor) {
  DlDegrees degrees(180.0);
  EXPECT_EQ(degrees.radians(), kDlScalar_Pi);
  EXPECT_EQ(degrees.degrees(), 180.0);
}

TEST(DlAngleTest, RadiansToDegreesConversion) {
  DlRadians radians(M_PI);
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
  DlRadians radians(M_PI);
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

}  // namespace testing
}  // namespace flutter
