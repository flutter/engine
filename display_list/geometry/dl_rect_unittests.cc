// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_rect.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlRectTest, FRectEmptyDeclaration) {
  DlFRect rect;

  ASSERT_EQ(rect.left(), 0.0f);
  ASSERT_EQ(rect.top(), 0.0f);
  ASSERT_EQ(rect.right(), 0.0f);
  ASSERT_EQ(rect.bottom(), 0.0f);
  ASSERT_EQ(rect.x(), 0.0f);
  ASSERT_EQ(rect.y(), 0.0f);
  ASSERT_EQ(rect.width(), 0.0f);
  ASSERT_EQ(rect.height(), 0.0f);
  ASSERT_TRUE(rect.is_empty());
  ASSERT_TRUE(rect.is_finite());
}

TEST(DlRectTest, IRectEmptyDeclaration) {
  DlIRect rect;

  ASSERT_EQ(rect.left(), 0);
  ASSERT_EQ(rect.top(), 0);
  ASSERT_EQ(rect.right(), 0);
  ASSERT_EQ(rect.bottom(), 0);
  ASSERT_EQ(rect.x(), 0);
  ASSERT_EQ(rect.y(), 0);
  ASSERT_EQ(rect.width(), 0u);
  ASSERT_EQ(rect.height(), 0u);
  ASSERT_TRUE(rect.is_empty());
  // ASSERT_TRUE(rect.is_finite());  // should fail to compile
}

TEST(DlRectTest, FRectDefaultDeclaration) {
  DlFRect rect = DlFRect();

  ASSERT_EQ(rect.left(), 0.0f);
  ASSERT_EQ(rect.top(), 0.0f);
  ASSERT_EQ(rect.right(), 0.0f);
  ASSERT_EQ(rect.bottom(), 0.0f);
  ASSERT_EQ(rect.x(), 0.0f);
  ASSERT_EQ(rect.y(), 0.0f);
  ASSERT_EQ(rect.width(), 0.0f);
  ASSERT_EQ(rect.height(), 0.0f);
  ASSERT_TRUE(rect.is_empty());
  ASSERT_TRUE(rect.is_finite());
}

TEST(DlRectTest, IRectDefaultDeclaration) {
  DlIRect rect = DlIRect();

  ASSERT_EQ(rect.left(), 0);
  ASSERT_EQ(rect.top(), 0);
  ASSERT_EQ(rect.right(), 0);
  ASSERT_EQ(rect.bottom(), 0);
  ASSERT_EQ(rect.x(), 0);
  ASSERT_EQ(rect.y(), 0);
  ASSERT_EQ(rect.width(), 0u);
  ASSERT_EQ(rect.height(), 0u);
  ASSERT_TRUE(rect.is_empty());
}

TEST(DlRectTest, FRectSimple) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);

  ASSERT_EQ(rect.left(), 5.125f);
  ASSERT_EQ(rect.top(), 10.25f);
  ASSERT_EQ(rect.right(), 20.625f);
  ASSERT_EQ(rect.bottom(), 25.375f);
  ASSERT_EQ(rect.x(), 5.125f);
  ASSERT_EQ(rect.y(), 10.25f);
  ASSERT_EQ(rect.width(), 15.5f);
  ASSERT_EQ(rect.height(), 15.125f);
  ASSERT_FALSE(rect.is_empty());
  ASSERT_TRUE(rect.is_finite());
}

TEST(DlRectTest, IRectSimple) {
  DlIRect rect = DlIRect::MakeLTRB(5, 10, 20, 25);

  ASSERT_EQ(rect.left(), 5);
  ASSERT_EQ(rect.top(), 10);
  ASSERT_EQ(rect.right(), 20);
  ASSERT_EQ(rect.bottom(), 25);
  ASSERT_EQ(rect.x(), 5);
  ASSERT_EQ(rect.y(), 10);
  ASSERT_EQ(rect.width(), 15u);
  ASSERT_EQ(rect.height(), 15u);
  ASSERT_FALSE(rect.is_empty());
}

TEST(DlRectTest, FRectRoundingEmpty) {
  DlFRect rect;

  ASSERT_EQ(DlFRect::MakeRoundedOut(rect), DlFRect());
  ASSERT_EQ(DlFRect::MakeRoundedIn(rect), DlFRect());
  ASSERT_EQ(DlFRect::MakeRounded(rect), DlFRect());

  ASSERT_EQ(DlIRect::MakeRoundedOut(rect), DlIRect());
  ASSERT_EQ(DlIRect::MakeRoundedIn(rect), DlIRect());
  ASSERT_EQ(DlIRect::MakeRounded(rect), DlIRect());

  ASSERT_EQ(rect.RoundedOut(), DlFRect());
  ASSERT_EQ(rect.RoundedIn(), DlFRect());
  ASSERT_EQ(rect.Rounded(), DlFRect());
}

TEST(DlRectTest, FRectRoundingSimple) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);

  ASSERT_EQ(DlFRect::MakeRoundedOut(rect), DlFRect::MakeLTRB(5, 10, 21, 26));
  ASSERT_EQ(DlFRect::MakeRoundedIn(rect), DlFRect::MakeLTRB(6, 11, 20, 25));
  ASSERT_EQ(DlFRect::MakeRounded(rect), DlFRect::MakeLTRB(5, 10, 21, 25));

  ASSERT_EQ(DlIRect::MakeRoundedOut(rect), DlIRect::MakeLTRB(5, 10, 21, 26));
  ASSERT_EQ(DlIRect::MakeRoundedIn(rect), DlIRect::MakeLTRB(6, 11, 20, 25));
  ASSERT_EQ(DlIRect::MakeRounded(rect), DlIRect::MakeLTRB(5, 10, 21, 25));

  ASSERT_EQ(rect.RoundedOut(), DlFRect::MakeLTRB(5.0f, 10.0f, 21.0f, 26.0f));
  ASSERT_EQ(rect.RoundedIn(), DlFRect::MakeLTRB(6.0f, 11.0f, 20.0f, 25.0f));
  ASSERT_EQ(rect.Rounded(), DlFRect::MakeLTRB(5.0f, 10.0f, 21.0f, 25.0f));
}

TEST(DlRectTest, FRectCopy) {
  // Using fractional-power-of-2 friendly values for equality tests
  DlFRect rect = DlFRect::MakeLTRB(5.125f, 10.25f, 20.625f, 25.375f);
  DlFRect copy = rect;

  ASSERT_EQ(rect, copy);
  ASSERT_EQ(copy.left(), 5.125f);
  ASSERT_EQ(copy.top(), 10.25f);
  ASSERT_EQ(copy.right(), 20.625f);
  ASSERT_EQ(copy.bottom(), 25.375f);
  ASSERT_EQ(copy.x(), 5.125f);
  ASSERT_EQ(copy.y(), 10.25f);
  ASSERT_EQ(copy.width(), 15.5f);
  ASSERT_EQ(copy.height(), 15.125f);
  ASSERT_FALSE(copy.is_empty());
  ASSERT_TRUE(copy.is_finite());
}

TEST(DlRectTest, IRectCopy) {
  DlIRect rect = DlIRect::MakeLTRB(5, 10, 20, 25);
  DlIRect copy = rect;

  ASSERT_EQ(rect, copy);
  ASSERT_EQ(copy.left(), 5);
  ASSERT_EQ(copy.top(), 10);
  ASSERT_EQ(copy.right(), 20);
  ASSERT_EQ(copy.bottom(), 25);
  ASSERT_EQ(copy.x(), 5);
  ASSERT_EQ(copy.y(), 10);
  ASSERT_EQ(copy.width(), 15u);
  ASSERT_EQ(copy.height(), 15u);
  ASSERT_FALSE(copy.is_empty());
}

TEST(DlRectTest, IRectDoesNotIntersectEmpty) {
  DlIRect rect = DlIRect::MakeLTRB(50, 50, 100, 100);

  auto test = [&rect](DlInt l, DlInt t, DlInt r, DlInt b,
                      const std::string& label) {
    EXPECT_FALSE(rect.Intersects(DlIRect::MakeLTRB(l, b, r, t)))
        << label << " with Top/Bottom swapped";
    EXPECT_FALSE(rect.Intersects(DlIRect::MakeLTRB(r, b, l, t)))
        << label << " with Left/Right swapped";
    EXPECT_FALSE(rect.Intersects(DlIRect::MakeLTRB(r, t, l, b)))
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

  test(70, 80, 70, 80, "Inside");

  test(40, 70, 60, 80, "Straddling Left");
  test(70, 40, 80, 60, "Straddling Top");
  test(90, 70, 110, 80, "Straddling Right");
  test(70, 90, 80, 110, "Straddling Bottom");
}

TEST(DlRectTest, FRectDoesNotIntersectEmpty) {
  DlFRect rect = DlFRect::MakeLTRB(50, 50, 100, 100);

  auto test = [&rect](DlScalar l, DlScalar t, DlScalar r, DlScalar b,
                      const std::string& label) {
    EXPECT_FALSE(rect.Intersects(DlFRect::MakeLTRB(l, b, r, t)))
        << label << " with Top/Bottom swapped";
    EXPECT_FALSE(rect.Intersects(DlFRect::MakeLTRB(r, b, l, t)))
        << label << " with Left/Right swapped";
    EXPECT_FALSE(rect.Intersects(DlFRect::MakeLTRB(r, t, l, b)))
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

  test(70, 80, 70, 80, "Inside");

  test(40, 70, 60, 80, "Straddling Left");
  test(70, 40, 80, 60, "Straddling Top");
  test(90, 70, 110, 80, "Straddling Right");
  test(70, 90, 80, 110, "Straddling Bottom");
}

TEST(DlRectTest, EmptyIRectDoesNotIntersect) {
  DlIRect rect = DlIRect::MakeLTRB(50, 50, 100, 100);

  auto test = [&rect](DlInt l, DlInt t, DlInt r, DlInt b,
                      const std::string& label) {
    EXPECT_FALSE(DlIRect::MakeLTRB(l, b, r, t).Intersects(rect))
        << label << " with Top/Bottom swapped";
    EXPECT_FALSE(DlIRect::MakeLTRB(r, b, l, t).Intersects(rect))
        << label << " with Left/Right swapped";
    EXPECT_FALSE(DlIRect::MakeLTRB(r, t, l, b).Intersects(rect))
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

  test(70, 80, 70, 80, "Inside");

  test(40, 70, 60, 80, "Straddling Left");
  test(70, 40, 80, 60, "Straddling Top");
  test(90, 70, 110, 80, "Straddling Right");
  test(70, 90, 80, 110, "Straddling Bottom");
}

TEST(DlRectTest, EmptyFRectDoesNotIntersect) {
  DlFRect rect = DlFRect::MakeLTRB(50, 50, 100, 100);

  auto test = [&rect](DlScalar l, DlScalar t, DlScalar r, DlScalar b,
                      const std::string& label) {
    EXPECT_FALSE(DlFRect::MakeLTRB(l, b, r, t).Intersects(rect))
        << label << " with Top/Bottom swapped";
    EXPECT_FALSE(DlFRect::MakeLTRB(r, b, l, t).Intersects(rect))
        << label << " with Left/Right swapped";
    EXPECT_FALSE(DlFRect::MakeLTRB(r, t, l, b).Intersects(rect))
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

  test(70, 80, 70, 80, "Inside");

  test(40, 70, 60, 80, "Straddling Left");
  test(70, 40, 80, 60, "Straddling Top");
  test(90, 70, 110, 80, "Straddling Right");
  test(70, 90, 80, 110, "Straddling Bottom");
}

}  // namespace testing
}  // namespace flutter
