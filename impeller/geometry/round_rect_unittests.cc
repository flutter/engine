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

TEST(RoundRectTest, RoundingRadiiPartialNamedSizesConstructor) {
  {
    RoundingRadii radii = {
        .top_left = Size(5.0f, 5.5f),
    };

    EXPECT_FALSE(radii.AreAllEmpty());
    EXPECT_FALSE(radii.AreAllSame());
    EXPECT_TRUE(radii.IsFinite());
    EXPECT_EQ(radii.top_left, Size(5.0f, 5.5f));
    EXPECT_EQ(radii.top_right, Size());
    EXPECT_EQ(radii.bottom_left, Size());
    EXPECT_EQ(radii.bottom_right, Size());
  }

  {
    RoundingRadii radii = {
        .top_right = Size(6.0f, 6.5f),
    };

    EXPECT_FALSE(radii.AreAllEmpty());
    EXPECT_FALSE(radii.AreAllSame());
    EXPECT_TRUE(radii.IsFinite());
    EXPECT_EQ(radii.top_left, Size());
    EXPECT_EQ(radii.top_right, Size(6.0f, 6.5f));
    EXPECT_EQ(radii.bottom_left, Size());
    EXPECT_EQ(radii.bottom_right, Size());
  }

  {
    RoundingRadii radii = {
        .bottom_left = Size(7.0f, 7.5f),
    };

    EXPECT_FALSE(radii.AreAllEmpty());
    EXPECT_FALSE(radii.AreAllSame());
    EXPECT_TRUE(radii.IsFinite());
    EXPECT_EQ(radii.top_left, Size());
    EXPECT_EQ(radii.top_right, Size());
    EXPECT_EQ(radii.bottom_left, Size(7.0f, 7.5f));
    EXPECT_EQ(radii.bottom_right, Size());
  }

  {
    RoundingRadii radii = {
        .bottom_right = Size(8.0f, 8.5f),
    };

    EXPECT_FALSE(radii.AreAllEmpty());
    EXPECT_FALSE(radii.AreAllSame());
    EXPECT_TRUE(radii.IsFinite());
    EXPECT_EQ(radii.top_left, Size());
    EXPECT_EQ(radii.top_right, Size());
    EXPECT_EQ(radii.bottom_left, Size());
    EXPECT_EQ(radii.bottom_right, Size(8.0f, 8.5f));
  }
}

TEST(RoundRectTest, RoundingRadiiMultiply) {
  RoundingRadii radii = {
      .top_left = Size(5.0f, 5.5f),
      .top_right = Size(6.0f, 6.5f),
      .bottom_left = Size(7.0f, 7.5f),
      .bottom_right = Size(8.0f, 8.5f),
  };
  RoundingRadii doubled = radii * 2.0f;

  EXPECT_FALSE(doubled.AreAllEmpty());
  EXPECT_FALSE(doubled.AreAllSame());
  EXPECT_TRUE(doubled.IsFinite());
  EXPECT_EQ(doubled.top_left, Size(10.0f, 11.0f));
  EXPECT_EQ(doubled.top_right, Size(12.0f, 13.0f));
  EXPECT_EQ(doubled.bottom_left, Size(14.0f, 15.0f));
  EXPECT_EQ(doubled.bottom_right, Size(16.0f, 17.0f));
}

TEST(RoundRectTest, RoundingRadiiEquals) {
  RoundingRadii radii = {
      .top_left = Size(5.0f, 5.5f),
      .top_right = Size(6.0f, 6.5f),
      .bottom_left = Size(7.0f, 7.5f),
      .bottom_right = Size(8.0f, 8.5f),
  };
  RoundingRadii other = {
      .top_left = Size(5.0f, 5.5f),
      .top_right = Size(6.0f, 6.5f),
      .bottom_left = Size(7.0f, 7.5f),
      .bottom_right = Size(8.0f, 8.5f),
  };

  EXPECT_EQ(radii, other);
}

TEST(RoundRectTest, RoundingRadiiNotEquals) {
  const RoundingRadii radii = {
      .top_left = Size(5.0f, 5.5f),
      .top_right = Size(6.0f, 6.5f),
      .bottom_left = Size(7.0f, 7.5f),
      .bottom_right = Size(8.0f, 8.5f),
  };

  {
    RoundingRadii different = radii;
    different.top_left.width = 100.0f;
    EXPECT_NE(different, radii);
  }
  {
    RoundingRadii different = radii;
    different.top_left.height = 100.0f;
    EXPECT_NE(different, radii);
  }
  {
    RoundingRadii different = radii;
    different.top_right.width = 100.0f;
    EXPECT_NE(different, radii);
  }
  {
    RoundingRadii different = radii;
    different.top_right.height = 100.0f;
    EXPECT_NE(different, radii);
  }
  {
    RoundingRadii different = radii;
    different.bottom_left.width = 100.0f;
    EXPECT_NE(different, radii);
  }
  {
    RoundingRadii different = radii;
    different.bottom_left.height = 100.0f;
    EXPECT_NE(different, radii);
  }
  {
    RoundingRadii different = radii;
    different.bottom_right.width = 100.0f;
    EXPECT_NE(different, radii);
  }
  {
    RoundingRadii different = radii;
    different.bottom_right.height = 100.0f;
    EXPECT_NE(different, radii);
  }
}

TEST(RoundRectTest, EmptyDeclaration) {
  RoundRect round_rect;

  EXPECT_TRUE(round_rect.IsEmpty());
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
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
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_TRUE(round_rect.GetBounds().IsEmpty());
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
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_TRUE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect());
  EXPECT_EQ(round_rect.GetRadii().top_left, Size());
  EXPECT_EQ(round_rect.GetRadii().top_right, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size());
}

TEST(RoundRectTest, RectConstructor) {
  RoundRect round_rect =
      RoundRect::MakeRect(Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_TRUE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size());
  EXPECT_EQ(round_rect.GetRadii().top_right, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size());
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size());
}

TEST(RoundRectTest, OvalConstructor) {
  RoundRect round_rect =
      RoundRect::MakeOval(Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_TRUE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size(5.0f, 5.0f));
  EXPECT_EQ(round_rect.GetRadii().top_right, Size(5.0f, 5.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size(5.0f, 5.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size(5.0f, 5.0f));
}

TEST(RoundRectTest, RectRadiusConstructor) {
  RoundRect round_rect = RoundRect::MakeRectRadius(
      Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f), 2.0f);

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size(2.0f, 2.0f));
  EXPECT_EQ(round_rect.GetRadii().top_right, Size(2.0f, 2.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size(2.0f, 2.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size(2.0f, 2.0f));
}

TEST(RoundRectTest, RectXYConstructor) {
  RoundRect round_rect = RoundRect::MakeRectXY(
      Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f), 2.0f, 3.0f);

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size(2.0f, 3.0f));
  EXPECT_EQ(round_rect.GetRadii().top_right, Size(2.0f, 3.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size(2.0f, 3.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size(2.0f, 3.0f));
}

TEST(RoundRectTest, RectSizeConstructor) {
  RoundRect round_rect = RoundRect::MakeRectXY(
      Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f), Size(2.0f, 3.0f));

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size(2.0f, 3.0f));
  EXPECT_EQ(round_rect.GetRadii().top_right, Size(2.0f, 3.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size(2.0f, 3.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size(2.0f, 3.0f));
}

TEST(RoundRectTest, RectRadiiConstructor) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f),
                               {
                                   .top_left = Size(1.0, 1.5),
                                   .top_right = Size(2.0, 2.5f),
                                   .bottom_left = Size(3.0, 3.5f),
                                   .bottom_right = Size(4.0, 4.5f),
                               });

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 20.0f, 20.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size(1.0f, 1.5f));
  EXPECT_EQ(round_rect.GetRadii().top_right, Size(2.0f, 2.5f));
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size(3.0f, 3.5f));
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size(4.0f, 4.5f));
}

TEST(RoundRectTest, RectRadiiOverflowWidthConstructor) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 6.0f, 30.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  // Largest sum of paired radii widths is the bottom edge which sums to 12
  // Rect is only 6 wide so all radii are scaled by half
  // Rect is 30 tall so no scaling should happen due to radii heights

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 16.0f, 40.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size(0.5f, 1.0f));
  EXPECT_EQ(round_rect.GetRadii().top_right, Size(1.5f, 2.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size(2.5f, 3.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size(3.5f, 4.0f));
}

TEST(RoundRectTest, RectRadiiOverflowHeightConstructor) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 30.0f, 6.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  // Largest sum of paired radii heights is the right edge which sums to 12
  // Rect is only 6 tall so all radii are scaled by half
  // Rect is 30 wide so no scaling should happen due to radii widths

  EXPECT_FALSE(round_rect.IsEmpty());
  EXPECT_FALSE(round_rect.IsRect());
  EXPECT_FALSE(round_rect.IsOval());
  EXPECT_TRUE(round_rect.IsFinite());
  EXPECT_FALSE(round_rect.GetBounds().IsEmpty());
  EXPECT_EQ(round_rect.GetBounds(), Rect::MakeLTRB(10.0f, 10.0f, 40.0f, 16.0f));
  EXPECT_EQ(round_rect.GetRadii().top_left, Size(0.5f, 1.0f));
  EXPECT_EQ(round_rect.GetRadii().top_right, Size(1.5f, 2.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_left, Size(2.5f, 3.0f));
  EXPECT_EQ(round_rect.GetRadii().bottom_right, Size(3.5f, 4.0f));
}

TEST(RoundRectTest, Shift) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 30.0f, 30.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  RoundRect shifted = round_rect.Shift(5.0, 6.0);

  EXPECT_FALSE(shifted.IsEmpty());
  EXPECT_FALSE(shifted.IsRect());
  EXPECT_FALSE(shifted.IsOval());
  EXPECT_TRUE(shifted.IsFinite());
  EXPECT_FALSE(shifted.GetBounds().IsEmpty());
  EXPECT_EQ(shifted.GetBounds(), Rect::MakeLTRB(15.0f, 16.0f, 45.0f, 46.0f));
  EXPECT_EQ(shifted.GetRadii().top_left, Size(1.0f, 2.0f));
  EXPECT_EQ(shifted.GetRadii().top_right, Size(3.0f, 4.0f));
  EXPECT_EQ(shifted.GetRadii().bottom_left, Size(5.0f, 6.0f));
  EXPECT_EQ(shifted.GetRadii().bottom_right, Size(7.0f, 8.0f));

  EXPECT_EQ(shifted,
            RoundRect::MakeRectRadii(Rect::MakeXYWH(15.0f, 16.0f, 30.0f, 30.0f),
                                     {
                                         .top_left = Size(1.0f, 2.0f),
                                         .top_right = Size(3.0f, 4.0f),
                                         .bottom_left = Size(5.0f, 6.0f),
                                         .bottom_right = Size(7.0f, 8.0f),
                                     }));
}

TEST(RoundRectTest, ExpandScalar) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 30.0f, 30.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  RoundRect expanded = round_rect.Expand(5.0);

  EXPECT_FALSE(expanded.IsEmpty());
  EXPECT_FALSE(expanded.IsRect());
  EXPECT_FALSE(expanded.IsOval());
  EXPECT_TRUE(expanded.IsFinite());
  EXPECT_FALSE(expanded.GetBounds().IsEmpty());
  EXPECT_EQ(expanded.GetBounds(), Rect::MakeLTRB(5.0f, 5.0f, 45.0f, 45.0f));
  EXPECT_EQ(expanded.GetRadii().top_left, Size(1.0f, 2.0f));
  EXPECT_EQ(expanded.GetRadii().top_right, Size(3.0f, 4.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_left, Size(5.0f, 6.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_right, Size(7.0f, 8.0f));

  EXPECT_EQ(expanded,
            RoundRect::MakeRectRadii(Rect::MakeXYWH(5.0f, 5.0f, 40.0f, 40.0f),
                                     {
                                         .top_left = Size(1.0f, 2.0f),
                                         .top_right = Size(3.0f, 4.0f),
                                         .bottom_left = Size(5.0f, 6.0f),
                                         .bottom_right = Size(7.0f, 8.0f),
                                     }));
}

TEST(RoundRectTest, ExpandTwoScalars) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 30.0f, 30.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  RoundRect expanded = round_rect.Expand(5.0, 6.0);

  EXPECT_FALSE(expanded.IsEmpty());
  EXPECT_FALSE(expanded.IsRect());
  EXPECT_FALSE(expanded.IsOval());
  EXPECT_TRUE(expanded.IsFinite());
  EXPECT_FALSE(expanded.GetBounds().IsEmpty());
  EXPECT_EQ(expanded.GetBounds(), Rect::MakeLTRB(5.0f, 4.0f, 45.0f, 46.0f));
  EXPECT_EQ(expanded.GetRadii().top_left, Size(1.0f, 2.0f));
  EXPECT_EQ(expanded.GetRadii().top_right, Size(3.0f, 4.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_left, Size(5.0f, 6.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_right, Size(7.0f, 8.0f));

  EXPECT_EQ(expanded,
            RoundRect::MakeRectRadii(Rect::MakeXYWH(5.0f, 4.0f, 40.0f, 42.0f),
                                     {
                                         .top_left = Size(1.0f, 2.0f),
                                         .top_right = Size(3.0f, 4.0f),
                                         .bottom_left = Size(5.0f, 6.0f),
                                         .bottom_right = Size(7.0f, 8.0f),
                                     }));
}

TEST(RoundRectTest, ExpandFourScalars) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 30.0f, 30.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  RoundRect expanded = round_rect.Expand(5.0, 6.0, 7.0, 8.0);

  EXPECT_FALSE(expanded.IsEmpty());
  EXPECT_FALSE(expanded.IsRect());
  EXPECT_FALSE(expanded.IsOval());
  EXPECT_TRUE(expanded.IsFinite());
  EXPECT_FALSE(expanded.GetBounds().IsEmpty());
  EXPECT_EQ(expanded.GetBounds(), Rect::MakeLTRB(5.0f, 4.0f, 47.0f, 48.0f));
  EXPECT_EQ(expanded.GetRadii().top_left, Size(1.0f, 2.0f));
  EXPECT_EQ(expanded.GetRadii().top_right, Size(3.0f, 4.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_left, Size(5.0f, 6.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_right, Size(7.0f, 8.0f));

  EXPECT_EQ(expanded,
            RoundRect::MakeRectRadii(Rect::MakeXYWH(5.0f, 4.0f, 42.0f, 44.0f),
                                     {
                                         .top_left = Size(1.0f, 2.0f),
                                         .top_right = Size(3.0f, 4.0f),
                                         .bottom_left = Size(5.0f, 6.0f),
                                         .bottom_right = Size(7.0f, 8.0f),
                                     }));
}

TEST(RoundRectTest, ContractScalar) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 30.0f, 30.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  RoundRect expanded = round_rect.Expand(-2.0);

  EXPECT_FALSE(expanded.IsEmpty());
  EXPECT_FALSE(expanded.IsRect());
  EXPECT_FALSE(expanded.IsOval());
  EXPECT_TRUE(expanded.IsFinite());
  EXPECT_FALSE(expanded.GetBounds().IsEmpty());
  EXPECT_EQ(expanded.GetBounds(), Rect::MakeLTRB(12.0f, 12.0f, 38.0f, 38.0f));
  EXPECT_EQ(expanded.GetRadii().top_left, Size(1.0f, 2.0f));
  EXPECT_EQ(expanded.GetRadii().top_right, Size(3.0f, 4.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_left, Size(5.0f, 6.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_right, Size(7.0f, 8.0f));

  EXPECT_EQ(expanded,
            RoundRect::MakeRectRadii(Rect::MakeXYWH(12.0f, 12.0f, 26.0f, 26.0f),
                                     {
                                         .top_left = Size(1.0f, 2.0f),
                                         .top_right = Size(3.0f, 4.0f),
                                         .bottom_left = Size(5.0f, 6.0f),
                                         .bottom_right = Size(7.0f, 8.0f),
                                     }));
}

TEST(RoundRectTest, ContractTwoScalars) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 30.0f, 30.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  RoundRect expanded = round_rect.Expand(-1.0, -2.0);

  EXPECT_FALSE(expanded.IsEmpty());
  EXPECT_FALSE(expanded.IsRect());
  EXPECT_FALSE(expanded.IsOval());
  EXPECT_TRUE(expanded.IsFinite());
  EXPECT_FALSE(expanded.GetBounds().IsEmpty());
  EXPECT_EQ(expanded.GetBounds(), Rect::MakeLTRB(11.0f, 12.0f, 39.0f, 38.0f));
  EXPECT_EQ(expanded.GetRadii().top_left, Size(1.0f, 2.0f));
  EXPECT_EQ(expanded.GetRadii().top_right, Size(3.0f, 4.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_left, Size(5.0f, 6.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_right, Size(7.0f, 8.0f));

  EXPECT_EQ(expanded,
            RoundRect::MakeRectRadii(Rect::MakeXYWH(11.0f, 12.0f, 28.0f, 26.0f),
                                     {
                                         .top_left = Size(1.0f, 2.0f),
                                         .top_right = Size(3.0f, 4.0f),
                                         .bottom_left = Size(5.0f, 6.0f),
                                         .bottom_right = Size(7.0f, 8.0f),
                                     }));
}

TEST(RoundRectTest, ContractFourScalars) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 30.0f, 30.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  RoundRect expanded = round_rect.Expand(-1.0, -1.5, -2.0, -2.5);

  EXPECT_FALSE(expanded.IsEmpty());
  EXPECT_FALSE(expanded.IsRect());
  EXPECT_FALSE(expanded.IsOval());
  EXPECT_TRUE(expanded.IsFinite());
  EXPECT_FALSE(expanded.GetBounds().IsEmpty());
  EXPECT_EQ(expanded.GetBounds(), Rect::MakeLTRB(11.0f, 11.5f, 38.0f, 37.5f));
  EXPECT_EQ(expanded.GetRadii().top_left, Size(1.0f, 2.0f));
  EXPECT_EQ(expanded.GetRadii().top_right, Size(3.0f, 4.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_left, Size(5.0f, 6.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_right, Size(7.0f, 8.0f));

  EXPECT_EQ(expanded,
            RoundRect::MakeRectRadii(Rect::MakeXYWH(11.0f, 11.5f, 27.0f, 26.0f),
                                     {
                                         .top_left = Size(1.0f, 2.0f),
                                         .top_right = Size(3.0f, 4.0f),
                                         .bottom_left = Size(5.0f, 6.0f),
                                         .bottom_right = Size(7.0f, 8.0f),
                                     }));
}

TEST(RoundRectTest, ContractAndRequireRadiiAdjustment) {
  RoundRect round_rect =
      RoundRect::MakeRectRadii(Rect::MakeXYWH(10.0f, 10.0f, 30.0f, 30.0f),
                               {
                                   .top_left = Size(1.0f, 2.0f),
                                   .top_right = Size(3.0f, 4.0f),
                                   .bottom_left = Size(5.0f, 6.0f),
                                   .bottom_right = Size(7.0f, 8.0f),
                               });
  RoundRect expanded = round_rect.Expand(-12.0);
  // Largest sum of paired radii sizes are the bottom and right edges
  // both of which sum to 12
  // Rect was 30x30 reduced by 12 on all sides leaving only 6x6, so all
  // radii are scaled by half to avoid overflowing the contracted rect

  EXPECT_FALSE(expanded.IsEmpty());
  EXPECT_FALSE(expanded.IsRect());
  EXPECT_FALSE(expanded.IsOval());
  EXPECT_TRUE(expanded.IsFinite());
  EXPECT_FALSE(expanded.GetBounds().IsEmpty());
  EXPECT_EQ(expanded.GetBounds(), Rect::MakeLTRB(22.0f, 22.0f, 28.0f, 28.0f));
  EXPECT_EQ(expanded.GetRadii().top_left, Size(0.5f, 1.0f));
  EXPECT_EQ(expanded.GetRadii().top_right, Size(1.5f, 2.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_left, Size(2.5f, 3.0f));
  EXPECT_EQ(expanded.GetRadii().bottom_right, Size(3.5f, 4.0f));

  // In this test, the MakeRectRadii constructor will make the same
  // adjustment to the radii that the Expand method applied.
  EXPECT_EQ(expanded,
            RoundRect::MakeRectRadii(Rect::MakeXYWH(22.0f, 22.0f, 6.0f, 6.0f),
                                     {
                                         .top_left = Size(1.0f, 2.0f),
                                         .top_right = Size(3.0f, 4.0f),
                                         .bottom_left = Size(5.0f, 6.0f),
                                         .bottom_right = Size(7.0f, 8.0f),
                                     }));

  // In this test, the arguments to the constructor supply the correctly
  // adjusted radii (though there is no real way to tell other than
  // the result is the same).
  EXPECT_EQ(expanded,
            RoundRect::MakeRectRadii(Rect::MakeXYWH(22.0f, 22.0f, 6.0f, 6.0f),
                                     {
                                         .top_left = Size(0.5f, 1.0f),
                                         .top_right = Size(1.5f, 2.0f),
                                         .bottom_left = Size(2.5f, 3.0f),
                                         .bottom_right = Size(3.5f, 4.0f),
                                     }));
}

TEST(RoundRectTest, RoundRectContains) {
  Rect bounds = Rect::MakeLTRB(-50.0f, -50.0f, 50.0f, 50.0f);

  // TBD - rectangles have half-in, half-out containment so we need
  // to be careful about testing containment of corners. These tests
  // are a work in progress.
  {
    // RRect of bounds with no corners contains corners just barely
    auto no_corners = RoundRect::MakeRectXY(bounds, 0.0f, 0.0f);
    EXPECT_TRUE(no_corners.Contains({-50, -50}));
    // EXPECT_TRUE(no_corners.Contains({-50, 50}));
    // EXPECT_TRUE(no_corners.Contains({50, -50}));
    // EXPECT_TRUE(no_corners.Contains({50, 50}));
    EXPECT_FALSE(no_corners.Contains({-50.1, -50}));
    EXPECT_FALSE(no_corners.Contains({-50, -50.1}));
    EXPECT_FALSE(no_corners.Contains({-50.1, 50}));
    EXPECT_FALSE(no_corners.Contains({-50, 50.1}));
    EXPECT_FALSE(no_corners.Contains({50.1, -50}));
    EXPECT_FALSE(no_corners.Contains({50, -50.1}));
    EXPECT_FALSE(no_corners.Contains({50.1, 50}));
    EXPECT_FALSE(no_corners.Contains({50, 50.1}));
  }

  {
    // RRect of bounds with even the tiniest corners does not contain corners
    auto tiny_corners = RoundRect::MakeRectXY(bounds, 0.01f, 0.01f);
    EXPECT_FALSE(tiny_corners.Contains({-50, -50}));
    EXPECT_FALSE(tiny_corners.Contains({-50, 50}));
    EXPECT_FALSE(tiny_corners.Contains({50, -50}));
    EXPECT_FALSE(tiny_corners.Contains({50, 50}));
  }

  {
    // Expanded by 2.0 and then with a corner of 2.0 obviously still
    // contains the corners
    auto expanded_2_r_2 = RoundRect::MakeRectXY(bounds.Expand(2.0), 2.0f, 2.0f);
    EXPECT_TRUE(expanded_2_r_2.Contains({-50, -50}));
    EXPECT_TRUE(expanded_2_r_2.Contains({-50, 50}));
    EXPECT_TRUE(expanded_2_r_2.Contains({50, -50}));
    EXPECT_TRUE(expanded_2_r_2.Contains({50, 50}));
  }

  // The corner point of the cull rect is at (c-2, c-2) relative to the
  // corner of the rrect bounds so we compute its disance to the center
  // of the circular part and compare it to the radius of the corner (c)
  // to find the corner radius where it will start to leave the rounded
  // rectangle:
  //
  //     +-----------      +
  //     |    __---^^      |
  //     |  +/-------  +   |
  //     |  / \        |   c
  //     | /|   \     c-2  |
  //     |/ |     \    |   |
  //     || |       *  +   +
  //
  // sqrt(2*(c-2)*(c-2)) > c
  // 2*(c-2)*(c-2) > c*c
  // 2*(cc - 4c + 4) > cc
  // 2cc - 8c + 8 > cc
  // cc - 8c + 8 > 0
  // c > 8 +/- sqrt(64 - 32) / 2
  // c > ~6.828
  // corners set to 6.82 should still cover the cull rect
  // EXPECT_TRUE(state.rrect_covers_cull(SkRRect::MakeRectXY(test, 6.82f, 6.82f)));
  // but corners set to 6.83 should not cover the cull rect
  // EXPECT_FALSE(
  //     state.rrect_covers_cull(SkRRect::MakeRectXY(test, 6.84f, 6.84f)));
}

}  // namespace testing
}  // namespace impeller
