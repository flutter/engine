// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"

#include "flutter/impeller/geometry/round_rect.h"

#include "flutter/impeller/geometry/geometry_asserts.h"

namespace impeller {
namespace testing {

TEST(RoundRectTest, RoundingRadiiEmptyDeclaration) {
  RoundingRadii radii;

  EXPECT_TRUE(radii.AreAllEmpty());
  EXPECT_TRUE(radii.AreAllSame());
  EXPECT_TRUE(radii.IsFinite());
  EXPECT_EQ(radii.top_left, Size());
  EXPECT_EQ(radii.top_right, Size());
  EXPECT_EQ(radii.bottom_left, Size());
  EXPECT_EQ(radii.bottom_right, Size());
  EXPECT_EQ(radii.top_left.width, 0.0f);
  EXPECT_EQ(radii.top_left.height, 0.0f);
  EXPECT_EQ(radii.top_right.width, 0.0f);
  EXPECT_EQ(radii.top_right.height, 0.0f);
  EXPECT_EQ(radii.bottom_left.width, 0.0f);
  EXPECT_EQ(radii.bottom_left.height, 0.0f);
  EXPECT_EQ(radii.bottom_right.width, 0.0f);
  EXPECT_EQ(radii.bottom_right.height, 0.0f);
}

TEST(RoundRectTest, RoundingRadiiDefaultConstructor) {
  RoundingRadii radii = RoundingRadii();

  EXPECT_TRUE(radii.AreAllEmpty());
  EXPECT_TRUE(radii.AreAllSame());
  EXPECT_TRUE(radii.IsFinite());
  EXPECT_EQ(radii.top_left, Size());
  EXPECT_EQ(radii.top_right, Size());
  EXPECT_EQ(radii.bottom_left, Size());
  EXPECT_EQ(radii.bottom_right, Size());
}

TEST(RoundRectTest, RoundingRadiiScalarConstructor) {
  RoundingRadii radii = RoundingRadii::MakeRadius(5.0f);

  EXPECT_FALSE(radii.AreAllEmpty());
  EXPECT_TRUE(radii.AreAllSame());
  EXPECT_TRUE(radii.IsFinite());
  EXPECT_EQ(radii.top_left, Size(5.0f, 5.0f));
  EXPECT_EQ(radii.top_right, Size(5.0f, 5.0f));
  EXPECT_EQ(radii.bottom_left, Size(5.0f, 5.0f));
  EXPECT_EQ(radii.bottom_right, Size(5.0f, 5.0f));
}

TEST(RoundRectTest, RoundingRadiiEmptyScalarConstructor) {
  RoundingRadii radii = RoundingRadii::MakeRadius(-5.0f);

  EXPECT_TRUE(radii.AreAllEmpty());
  EXPECT_TRUE(radii.AreAllSame());
  EXPECT_TRUE(radii.IsFinite());
  EXPECT_EQ(radii.top_left, Size(-5.0f, -5.0f));
  EXPECT_EQ(radii.top_right, Size(-5.0f, -5.0f));
  EXPECT_EQ(radii.bottom_left, Size(-5.0f, -5.0f));
  EXPECT_EQ(radii.bottom_right, Size(-5.0f, -5.0f));
}

TEST(RoundRectTest, RoundingRadiiSizeConstructor) {
  RoundingRadii radii = RoundingRadii::MakeRadii(Size(5.0f, 6.0f));

  EXPECT_FALSE(radii.AreAllEmpty());
  EXPECT_TRUE(radii.AreAllSame());
  EXPECT_TRUE(radii.IsFinite());
  EXPECT_EQ(radii.top_left, Size(5.0f, 6.0f));
  EXPECT_EQ(radii.top_right, Size(5.0f, 6.0f));
  EXPECT_EQ(radii.bottom_left, Size(5.0f, 6.0f));
  EXPECT_EQ(radii.bottom_right, Size(5.0f, 6.0f));
}

TEST(RoundRectTest, RoundingRadiiEmptySizeConstructor) {
  {
    RoundingRadii radii = RoundingRadii::MakeRadii(Size(-5.0f, 6.0f));

    EXPECT_TRUE(radii.AreAllEmpty());
    EXPECT_TRUE(radii.AreAllSame());
    EXPECT_TRUE(radii.IsFinite());
    EXPECT_EQ(radii.top_left, Size(-5.0f, 6.0f));
    EXPECT_EQ(radii.top_right, Size(-5.0f, 6.0f));
    EXPECT_EQ(radii.bottom_left, Size(-5.0f, 6.0f));
    EXPECT_EQ(radii.bottom_right, Size(-5.0f, 6.0f));
  }

  {
    RoundingRadii radii = RoundingRadii::MakeRadii(Size(5.0f, -6.0f));

    EXPECT_TRUE(radii.AreAllEmpty());
    EXPECT_TRUE(radii.AreAllSame());
    EXPECT_TRUE(radii.IsFinite());
    EXPECT_EQ(radii.top_left, Size(5.0f, -6.0f));
    EXPECT_EQ(radii.top_right, Size(5.0f, -6.0f));
    EXPECT_EQ(radii.bottom_left, Size(5.0f, -6.0f));
    EXPECT_EQ(radii.bottom_right, Size(5.0f, -6.0f));
  }
}

TEST(RoundRectTest, RoundingRadiiNamedSizesConstructor) {
  RoundingRadii radii = {
      .top_left = Size(5.0f, 5.5f),
      .top_right = Size(6.0f, 6.5f),
      .bottom_left = Size(7.0f, 7.5f),
      .bottom_right = Size(8.0f, 8.5f),
  };

  EXPECT_FALSE(radii.AreAllEmpty());
  EXPECT_FALSE(radii.AreAllSame());
  EXPECT_TRUE(radii.IsFinite());
  EXPECT_EQ(radii.top_left, Size(5.0f, 5.5f));
  EXPECT_EQ(radii.top_right, Size(6.0f, 6.5f));
  EXPECT_EQ(radii.bottom_left, Size(7.0f, 7.5f));
  EXPECT_EQ(radii.bottom_right, Size(8.0f, 8.5f));
}

TEST(RoundRectTest, EmptyDeclaration) {
  RoundRect round_rect;

  EXPECT_TRUE(round_rect.IsEmpty());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_TRUE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect());
  EXPECT_EQ(round_rect.GetBounds().GetLeft(), 0.0f);
  EXPECT_EQ(round_rect.GetBounds().GetTop(), 0.0f);
  EXPECT_EQ(round_rect.GetBounds().GetRight(), 0.0f);
  EXPECT_EQ(round_rect.GetBounds().GetBottom(), 0.0f);
  EXPECT_EQ(round_rect.GetRadii().top_left, Size());
  EXPECT_EQ(round_rect.GetRadii().top_right, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size());
  EXPECT_EQ(round_rect.GetRadii().top_left.width, 0.0f);
  EXPECT_EQ(round_rect.GetRadii().top_left.height, 0.0f);
  EXPECT_EQ(round_rect.GetRadii().top_right.width, 0.0f);
  EXPECT_EQ(round_rect.GetRadii().top_right.height, 0.0f);
  EXPECT_EQ(round_rect.GetRadii().bottom_left.width, 0.0f);
  EXPECT_EQ(round_rect.GetRadii().bottom_left.height, 0.0f);
  EXPECT_EQ(round_rect.GetRadii().bottom_right.width, 0.0f);
  EXPECT_EQ(round_rect.GetRadii().bottom_right.height, 0.0f);
}

TEST(RoundRectTest, DefaultConstructor) {
  RoundRect round_rect = RoundRect();

  EXPECT_TRUE(round_rect.IsEmpty());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_TRUE(round_rect.GetBounds().IsEmpty());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_EQ(round_rect.GetBounds(), Rect());
  EXPECT_EQ(round_rect.GetRadii().top_left, Size());
  EXPECT_EQ(round_rect.GetRadii().top_right, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size());
}

TEST(RoundRectTest, EmptyRectConstruction) {
  RoundRect round_rect = RoundRect::MakeRectXY(
      Rect::MakeLTRB(20.0f, 20.0f, 10.0f, 10.0f), 10.0f, 10.0f);

  EXPECT_TRUE(round_rect.IsEmpty());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_TRUE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect());
  EXPECT_EQ(round_rect.GetRadii().top_left, Size());
  EXPECT_EQ(round_rect.GetRadii().top_right, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size());
}

TEST(RoundRectTest, RectConstructor) {
  RoundRect round_rect = RoundRect::MakeRectXY(
      Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f), 2.0f, 2.0f);

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size(2.0f, 2.0f));
  EXPECT_EQ(round_rect.GetRadii().top_right, Size(2.0f, 2.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size(2.0f, 2.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size(2.0f, 2.0f));
}

TEST(RoundRectTest, OvalConstructor) {
  RoundRect round_rect =
      RoundRect::MakeOval(Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size(5.0f, 5.0f));
  EXPECT_EQ(round_rect.GetRadii().top_right, Size(5.0f, 5.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size(5.0f, 5.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size(5.0f, 5.0f));
}

}  // namespace testing
}  // namespace impeller
