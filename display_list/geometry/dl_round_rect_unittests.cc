// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_round_rect.h"

#include "fml/logging.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlRoundRectTest, FRRectEmptyDeclaration) {
  DlFRRect rrect;

  ASSERT_EQ(rrect.left(), 0.0f);
  ASSERT_EQ(rrect.top(), 0.0f);
  ASSERT_EQ(rrect.right(), 0.0f);
  ASSERT_EQ(rrect.bottom(), 0.0f);
  ASSERT_EQ(rrect.x(), 0.0f);
  ASSERT_EQ(rrect.y(), 0.0f);
  ASSERT_EQ(rrect.width(), 0.0f);
  ASSERT_EQ(rrect.height(), 0.0f);
  ASSERT_TRUE(rrect.is_finite());
  ASSERT_TRUE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kEmpty);
  ASSERT_EQ(rrect.upper_left_radii(), DlFVector());
  ASSERT_EQ(rrect.upper_right_radii(), DlFVector());
  ASSERT_EQ(rrect.lower_right_radii(), DlFVector());
  ASSERT_EQ(rrect.lower_left_radii(), DlFVector());
}

TEST(DlRoundRectTest, FRectDefaultConstructor) {
  DlFRRect rrect = DlFRRect();

  ASSERT_EQ(rrect.left(), 0.0f);
  ASSERT_EQ(rrect.top(), 0.0f);
  ASSERT_EQ(rrect.right(), 0.0f);
  ASSERT_EQ(rrect.bottom(), 0.0f);
  ASSERT_EQ(rrect.x(), 0.0f);
  ASSERT_EQ(rrect.y(), 0.0f);
  ASSERT_EQ(rrect.width(), 0.0f);
  ASSERT_EQ(rrect.height(), 0.0f);
  ASSERT_TRUE(rrect.is_finite());
  ASSERT_TRUE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kEmpty);
  ASSERT_EQ(rrect.upper_left_radii(), DlFVector());
  ASSERT_EQ(rrect.upper_right_radii(), DlFVector());
  ASSERT_EQ(rrect.lower_right_radii(), DlFVector());
  ASSERT_EQ(rrect.lower_left_radii(), DlFVector());
}

TEST(DlRoundRectTest, FRRectMakeRect) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFRRect rrect = DlFRRect::MakeRect(rect);

  ASSERT_EQ(rrect.left(), 5.125f);
  ASSERT_EQ(rrect.top(), 10.25f);
  ASSERT_EQ(rrect.right(), 20.625f);
  ASSERT_EQ(rrect.bottom(), 25.375f);
  ASSERT_EQ(rrect.x(), 5.125f);
  ASSERT_EQ(rrect.y(), 10.25f);
  ASSERT_EQ(rrect.width(), 15.5f);
  ASSERT_EQ(rrect.height(), 15.125f);
  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_TRUE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kRect);
  ASSERT_EQ(rrect.upper_left_radii(), DlFVector());
  ASSERT_EQ(rrect.upper_right_radii(), DlFVector());
  ASSERT_EQ(rrect.lower_right_radii(), DlFVector());
  ASSERT_EQ(rrect.lower_left_radii(), DlFVector());
}

TEST(DlRoundRectTest, FRRectMakeOval) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFRRect rrect = DlFRRect::MakeOval(rect);

  ASSERT_EQ(rrect.left(), 5.125f);
  ASSERT_EQ(rrect.top(), 10.25f);
  ASSERT_EQ(rrect.right(), 20.625f);
  ASSERT_EQ(rrect.bottom(), 25.375f);
  ASSERT_EQ(rrect.x(), 5.125f);
  ASSERT_EQ(rrect.y(), 10.25f);
  ASSERT_EQ(rrect.width(), 15.5f);
  ASSERT_EQ(rrect.height(), 15.125f);
  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_TRUE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kOval);
  ASSERT_EQ(rrect.upper_left_radii(), DlFVector(7.75f, 7.5625f));
  ASSERT_EQ(rrect.upper_right_radii(), DlFVector(7.75f, 7.5625f));
  ASSERT_EQ(rrect.lower_right_radii(), DlFVector(7.75f, 7.5625f));
  ASSERT_EQ(rrect.lower_left_radii(), DlFVector(7.75f, 7.5625f));
}

TEST(DlRoundRectTest, FRRectMakeXYRect) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  DlScalar dx = 1.125f;
  DlScalar dy = 2.0625f;

  auto check_rect = [](const DlFRRect& rrect) {
    ASSERT_TRUE(rrect.is_finite());
    ASSERT_FALSE(rrect.is_empty());
    ASSERT_TRUE(rrect.is_rect());
    ASSERT_FALSE(rrect.is_oval());
    ASSERT_FALSE(rrect.has_circular_corners());
    ASSERT_FALSE(rrect.has_oval_corners());
    ASSERT_FALSE(rrect.is_nine_patch());
    ASSERT_FALSE(rrect.is_complex());
    ASSERT_EQ(rrect.type(), DlFRRect::Type::kRect);
  };

  auto check_oval_corners = [](const DlFRRect& rrect) {
    ASSERT_TRUE(rrect.is_finite());
    ASSERT_FALSE(rrect.is_empty());
    ASSERT_FALSE(rrect.is_rect());
    ASSERT_FALSE(rrect.is_oval());
    ASSERT_FALSE(rrect.has_circular_corners());
    ASSERT_TRUE(rrect.has_oval_corners());
    ASSERT_FALSE(rrect.is_nine_patch());
    ASSERT_FALSE(rrect.is_complex());
    ASSERT_EQ(rrect.type(), DlFRRect::Type::kOvalCorners);
  };

  check_oval_corners(DlFRRect::MakeRectXY(rect, dx, dy));

  check_rect(DlFRRect::MakeRectXY(rect, 0.0f, dy));
  check_rect(DlFRRect::MakeRectXY(rect, dx, 0.0f));
  check_rect(DlFRRect::MakeRectXY(rect, 0.0f, 0.0f));

  check_rect(DlFRRect::MakeRectXY(rect, nan, dy));
  check_rect(DlFRRect::MakeRectXY(rect, dx, nan));
  check_rect(DlFRRect::MakeRectXY(rect, nan, nan));

  check_rect(DlFRRect::MakeRectXY(rect, -dx, dy));
  check_rect(DlFRRect::MakeRectXY(rect, dx, -dy));
  check_rect(DlFRRect::MakeRectXY(rect, -dx, -dy));
}

TEST(DlRoundRectTest, FRRectMakeXYOval) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlScalar w = rect.width();
  DlScalar h = rect.height();
  DlScalar hw = w * 0.5f;
  DlScalar hh = h * 0.5f;

  auto check_oval = [](const DlFRRect& rrect) {
    ASSERT_TRUE(rrect.is_finite());
    ASSERT_FALSE(rrect.is_empty());
    ASSERT_FALSE(rrect.is_rect());
    ASSERT_TRUE(rrect.is_oval());
    ASSERT_FALSE(rrect.has_circular_corners());
    ASSERT_FALSE(rrect.has_oval_corners());
    ASSERT_FALSE(rrect.is_nine_patch());
    ASSERT_FALSE(rrect.is_complex());
    ASSERT_EQ(rrect.type(), DlFRRect::Type::kOval);
  };

  auto check_oval_corners = [](const DlFRRect& rrect) {
    ASSERT_TRUE(rrect.is_finite());
    ASSERT_FALSE(rrect.is_empty());
    ASSERT_FALSE(rrect.is_rect());
    ASSERT_FALSE(rrect.is_oval());
    ASSERT_FALSE(rrect.has_circular_corners());
    ASSERT_TRUE(rrect.has_oval_corners());
    ASSERT_FALSE(rrect.is_nine_patch());
    ASSERT_FALSE(rrect.is_complex());
    ASSERT_EQ(rrect.type(), DlFRRect::Type::kOvalCorners);
  };

  // Using fractional-power-of-2 friendly values for more accurate scaling
  DlScalar scale_down = 63.0f / 64.0f;
  DlScalar scale_up = 65.0f / 64.0f;
  // To be classified as oval, the radii provided must be larger than half
  // the size of the rectangle evenly scaled compared to its w/h
  check_oval_corners(DlFRRect::MakeRectXY(rect, hw * scale_down, hh * scale_down));
  check_oval(DlFRRect::MakeRectXY(rect, hw, hh));
  check_oval(DlFRRect::MakeRectXY(rect, hw * scale_up, hh * scale_up));
  check_oval_corners(DlFRRect::MakeRectXY(rect, hw, h));
  check_oval_corners(DlFRRect::MakeRectXY(rect, w, hh));
  check_oval(DlFRRect::MakeRectXY(rect, w, h));
}

TEST(DlRoundRectTest, FRRectMakeXYCircularCorners) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFRRect rrect = DlFRRect::MakeRectXY(rect, 1.125f, 1.125f);

  ASSERT_EQ(rrect.left(), 5.125f);
  ASSERT_EQ(rrect.top(), 10.25f);
  ASSERT_EQ(rrect.right(), 20.625f);
  ASSERT_EQ(rrect.bottom(), 25.375f);
  ASSERT_EQ(rrect.x(), 5.125f);
  ASSERT_EQ(rrect.y(), 10.25f);
  ASSERT_EQ(rrect.width(), 15.5f);
  ASSERT_EQ(rrect.height(), 15.125f);
  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_TRUE(rrect.has_circular_corners());
  ASSERT_TRUE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kCircularCorners);
  ASSERT_EQ(rrect.upper_left_radii(), DlFVector(1.125f, 1.125f));
  ASSERT_EQ(rrect.upper_right_radii(), DlFVector(1.125f, 1.125f));
  ASSERT_EQ(rrect.lower_right_radii(), DlFVector(1.125f, 1.125f));
  ASSERT_EQ(rrect.lower_left_radii(), DlFVector(1.125f, 1.125f));
}

TEST(DlRoundRectTest, FRRectMakeXYOvalCorners) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFRRect rrect = DlFRRect::MakeRectXY(rect, 1.125f, 2.0625f);

  ASSERT_EQ(rrect.left(), 5.125f);
  ASSERT_EQ(rrect.top(), 10.25f);
  ASSERT_EQ(rrect.right(), 20.625f);
  ASSERT_EQ(rrect.bottom(), 25.375f);
  ASSERT_EQ(rrect.x(), 5.125f);
  ASSERT_EQ(rrect.y(), 10.25f);
  ASSERT_EQ(rrect.width(), 15.5f);
  ASSERT_EQ(rrect.height(), 15.125f);
  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_TRUE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kOvalCorners);
  ASSERT_EQ(rrect.upper_left_radii(), DlFVector(1.125f, 2.0625f));
  ASSERT_EQ(rrect.upper_right_radii(), DlFVector(1.125f, 2.0625f));
  ASSERT_EQ(rrect.lower_right_radii(), DlFVector(1.125f, 2.0625f));
  ASSERT_EQ(rrect.lower_left_radii(), DlFVector(1.125f, 2.0625f));
}

TEST(DlRoundRectTest, FRRectMakeRectRadiiEmpty) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 5.125f, 25.375f);
  DlFVector radii[4] = {
      {1.125f, 1.25f},
      {1.375f, 1.5f},
      {1.625f, 1.75f},
      {1.875f, 2.0f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(rect, radii);

  ASSERT_TRUE(rrect.is_finite());
  ASSERT_TRUE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kEmpty);
}

TEST(DlRoundRectTest, FRRectMakeRectRadiiRect) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFVector radii[4] = {
      {0.0f, 0.0f},
      {0.0f, 0.0f},
      {0.0f, 0.0f},
      {0.0f, 0.0f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(rect, radii);

  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_TRUE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kRect);
}

TEST(DlRoundRectTest, FRRectMakeRectRadiiOval) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFVector radii[4] = {
      {7.75f, 7.5625f},
      {7.75f, 7.5625f},
      {7.75f, 7.5625f},
      {7.75f, 7.5625f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(rect, radii);

  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_TRUE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kOval);
}

TEST(DlRoundRectTest, FRRectMakeRectRadiiCircularCorners) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFVector radii[4] = {
      {7.125f, 7.125f},
      {7.125f, 7.125f},
      {7.125f, 7.125f},
      {7.125f, 7.125f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(rect, radii);

  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_TRUE(rrect.has_circular_corners());
  ASSERT_TRUE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kCircularCorners);
}

TEST(DlRoundRectTest, FRRectMakeRectRadiiOvalCorners) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFVector radii[4] = {
      {7.125f, 7.25f},
      {7.125f, 7.25f},
      {7.125f, 7.25f},
      {7.125f, 7.25f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(rect, radii);

  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_TRUE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kOvalCorners);
}

TEST(DlRoundRectTest, FRRectMakeRectRadiiNinePatch) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFVector radii[4] = {
      {7.125f, 7.375f},
      {7.25f, 7.375f},
      {7.25f, 7.625f},
      {7.125f, 7.625f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(rect, radii);

  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_TRUE(rrect.is_nine_patch());
  ASSERT_FALSE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kNinePatch);
}

TEST(DlRoundRectTest, FRRectMakeRectRadiiComplex) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFVector radii[4] = {
      {1.125f, 1.25f},
      {1.375f, 1.5f},
      {1.625f, 1.75f},
      {1.875f, 2.0f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(rect, radii);

  ASSERT_EQ(rrect.left(), 5.125f);
  ASSERT_EQ(rrect.top(), 10.25f);
  ASSERT_EQ(rrect.right(), 20.625f);
  ASSERT_EQ(rrect.bottom(), 25.375f);
  ASSERT_EQ(rrect.x(), 5.125f);
  ASSERT_EQ(rrect.y(), 10.25f);
  ASSERT_EQ(rrect.width(), 15.5f);
  ASSERT_EQ(rrect.height(), 15.125f);
  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_TRUE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kComplex);
  ASSERT_EQ(rrect.upper_left_radii(), DlFVector(1.125f, 1.25f));
  ASSERT_EQ(rrect.upper_right_radii(), DlFVector(1.375f, 1.5f));
  ASSERT_EQ(rrect.lower_right_radii(), DlFVector(1.625f, 1.75f));
  ASSERT_EQ(rrect.lower_left_radii(), DlFVector(1.875f, 2.0f));
}

TEST(DlRoundRectTest, FRRectCopy) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFVector radii[4] = {
      {1.125f, 1.25f},
      {1.375f, 1.5f},
      {1.625f, 1.75f},
      {1.875f, 2.0f},
  };
  DlFRRect from = DlFRRect::MakeRectRadii(rect, radii);
  DlFRRect rrect = from;

  ASSERT_EQ(rrect.left(), 5.125f);
  ASSERT_EQ(rrect.top(), 10.25f);
  ASSERT_EQ(rrect.right(), 20.625f);
  ASSERT_EQ(rrect.bottom(), 25.375f);
  ASSERT_EQ(rrect.x(), 5.125f);
  ASSERT_EQ(rrect.y(), 10.25f);
  ASSERT_EQ(rrect.width(), 15.5f);
  ASSERT_EQ(rrect.height(), 15.125f);
  ASSERT_TRUE(rrect.is_finite());
  ASSERT_FALSE(rrect.is_empty());
  ASSERT_FALSE(rrect.is_rect());
  ASSERT_FALSE(rrect.is_oval());
  ASSERT_FALSE(rrect.has_circular_corners());
  ASSERT_FALSE(rrect.has_oval_corners());
  ASSERT_FALSE(rrect.is_nine_patch());
  ASSERT_TRUE(rrect.is_complex());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kComplex);
  ASSERT_EQ(rrect.upper_left_radii(), DlFVector(1.125f, 1.25f));
  ASSERT_EQ(rrect.upper_right_radii(), DlFVector(1.375f, 1.5f));
  ASSERT_EQ(rrect.lower_right_radii(), DlFVector(1.625f, 1.75f));
  ASSERT_EQ(rrect.lower_left_radii(), DlFVector(1.875f, 2.0f));
}

TEST(DlRoundRectTest, FRRectContainsPoint) {
  DlFRect bounds = DlFRect::MakeLTRB(50, 50, 100, 100);
  DlFVector radii[4] = {
      {0.5f, 0.75f},
      {0.625f, 0.375f},
      {0.875f, 0.25f},
      {0.125f, 1.125f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(bounds, radii);
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kComplex);

  // Corners not contained (due to rounding)
  ASSERT_FALSE(rrect.Contains({50, 50}));
  ASSERT_FALSE(rrect.Contains({50, 100}));
  ASSERT_FALSE(rrect.Contains({100, 100}));
  ASSERT_FALSE(rrect.Contains({100, 50}));

  // Just inside corners not contained (due to rounding radii)
  ASSERT_FALSE(rrect.Contains({50.0625f, 50.0625f}));
  ASSERT_FALSE(rrect.Contains({99.9375f, 50.0625f}));
  ASSERT_FALSE(rrect.Contains({99.9375f, 99.9375f}));
  ASSERT_FALSE(rrect.Contains({50.0625f, 99.9375f}));

  // Just inside radii is contained
  ASSERT_TRUE(rrect.Contains({50.5f, 50.5f}));
  ASSERT_TRUE(rrect.Contains({50.5f, 99.5f}));
  ASSERT_TRUE(rrect.Contains({99.5f, 99.5f}));
  ASSERT_TRUE(rrect.Contains({99.5f, 50.5f}));

  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  // NaN not contained
  ASSERT_TRUE(rrect.Contains({75, 75}));
  ASSERT_FALSE(rrect.Contains({75, nan}));
  ASSERT_FALSE(rrect.Contains({nan, nan}));
  ASSERT_FALSE(rrect.Contains({nan, 75}));
}

TEST(DlRoundRectTest, FRRectContainsRect) {
  DlFRect bounds = DlFRect::MakeLTRB(50, 50, 100, 100);
  DlFVector radii[4] = {
      {0.5f, 0.75f},
      {0.625f, 0.375f},
      {0.875f, 0.25f},
      {0.125f, 1.125f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(bounds, radii);
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kComplex);

  auto check_contains = [&rrect](DlScalar l, DlScalar t, DlScalar r, DlScalar b,
                                 const std::string& label) {
    EXPECT_TRUE(rrect.Contains(DlFRect::MakeLTRB(l, t, r, b)))
        << label << " with Top/Bottom swapped";
    EXPECT_FALSE(rrect.Contains(DlFRect::MakeLTRB(l, b, r, t)))
        << label << " with Top/Bottom swapped";
    EXPECT_FALSE(rrect.Contains(DlFRect::MakeLTRB(r, b, l, t)))
        << label << " with Left/Right swapped";
    EXPECT_FALSE(rrect.Contains(DlFRect::MakeLTRB(r, t, l, b)))
        << label << " with all sides swapped";
  };

  // Corners not contained (due to rounding)
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(50, 50, 75, 75)));
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(75, 50, 100, 75)));
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(75, 75, 100, 100)));
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(50, 75, 75, 100)));

  // Just inside corners not contained (due to rounding radii)
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(50.0625f, 50.0625f, 75, 75)));
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(75, 50.0625f, 99.9375f, 75)));
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(75, 75, 99.9375f, 99.9375f)));
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(50.0625f, 75, 75, 99.9375f)));

  // Just inside radii is contained
  check_contains(50.5f, 50.5f, 75, 75, "Just inside Upper Left");
  check_contains(75, 50.5f, 99.5f, 75, "Just inside Upper Right");
  check_contains(75, 75, 99.5f, 99.5f, "Just inside Lower Right");
  check_contains(50.5f, 75, 75, 99.5f, "Just inside Lower Left");

  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  // NaN not contained
  check_contains(70, 70, 80, 80, "example rect for NaN testing");
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(nan, 70, 80, 80)));
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(70, nan, 80, 80)));
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(70, 70, nan, 80)));
  ASSERT_FALSE(rrect.Contains(DlFRect::MakeLTRB(70, 70, 80, nan)));
}

TEST(DlRoundRectTest, FRRectDoesNotContainEmpty) {
  DlFRect bounds = DlFRect::MakeLTRB(50, 50, 100, 100);
  DlFVector radii[4] = {
      {0.5f, 0.75f},
      {0.625f, 0.375f},
      {0.875f, 0.25f},
      {0.125f, 1.125f},
  };
  DlFRRect rrect = DlFRRect::MakeRectRadii(bounds, radii);
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kComplex);

  auto test = [&rrect](DlScalar l, DlScalar t, DlScalar r, DlScalar b,
                       const std::string& label) {
    EXPECT_FALSE(rrect.Contains(DlFRect::MakeLTRB(l, b, r, t)))
        << label << " with Top/Bottom swapped";
    EXPECT_FALSE(rrect.Contains(DlFRect::MakeLTRB(r, b, l, t)))
        << label << " with Left/Right swapped";
    EXPECT_FALSE(rrect.Contains(DlFRect::MakeLTRB(r, t, l, b)))
        << label << " with all sides swapped";
  };

  test(20, 20, 30, 30, "Above and Left");
  test(70, 20, 80, 30, "Above");
  test(120, 20, 130, 30, "Above and Right");
  test(120, 70, 130, 80, "Right");
  test(120, 120, 130, 130, "Below and Right");
  test(70, 120, 80, 130, "Below");
  test(20, 120, 30, 130, "Below and Left");
  test(20, 70, 30, 80, "Left");

  test(70, 70, 80, 80, "Inside");

  test(40, 70, 60, 80, "Straddling Left");
  test(70, 40, 80, 60, "Straddling Top");
  test(90, 70, 110, 80, "Straddling Right");
  test(70, 90, 80, 110, "Straddling Bottom");
}

template <typename R>
static constexpr inline R flip_lr(R rect) {
  return R::MakeLTRB(rect.right(), rect.top(), rect.left(), rect.bottom());
}

template <typename R>
static constexpr inline R flip_tb(R rect) {
  return R::MakeLTRB(rect.left(), rect.bottom(), rect.right(), rect.top());
}

template <typename R>
static constexpr inline R flip_lrtb(R rect) {
  return flip_lr(flip_tb(rect));
}

TEST(DlRoundRectTest, EmptyFRectDoesNotContain) {
  // Since DlFRRect is self-sorting, the only truly empty configuration
  // is having zero dimensions
  DlFRRect rrect;
  ASSERT_TRUE(rrect.is_empty());
  ASSERT_EQ(rrect.type(), DlFRRect::Type::kEmpty);

  auto test = [&rrect](DlScalar l, DlScalar t, DlScalar r, DlScalar b,
                       const std::string& label) {
    DlFRect rect = DlFRect::MakeLTRB(l, t, r, b);
    EXPECT_FALSE(rrect.Contains(rect))  //
        << label << " with nothing swapped";
    EXPECT_FALSE(rrect.Contains(flip_tb(rect)))
        << label << " with Top/Bottom swapped";
    EXPECT_FALSE(rrect.Contains(flip_lr(rect)))
        << label << " with Left/Right swapped";
    EXPECT_FALSE(rrect.Contains(flip_lrtb(rect)))
        << label << " with all sides swapped";
  };

  test(20, 20, 30, 30, "Above and Left");
  test(70, 20, 80, 30, "Above");
  test(120, 20, 130, 30, "Above and Right");
  test(120, 70, 130, 80, "Right");
  test(120, 120, 130, 130, "Below and Right");
  test(70, 120, 80, 130, "Below");
  test(20, 120, 30, 130, "Below and Left");
  test(20, 70, 30, 80, "Left");

  test(70, 70, 80, 80, "Inside");

  test(40, 70, 60, 80, "Straddling Left");
  test(70, 40, 80, 60, "Straddling Top");
  test(90, 70, 110, 80, "Straddling Right");
  test(70, 90, 80, 110, "Straddling Bottom");
}

}  // namespace testing
}  // namespace flutter
