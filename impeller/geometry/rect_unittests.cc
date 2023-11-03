// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"

#include "flutter/impeller/geometry/rect.h"

#include "flutter/impeller/geometry/geometry_asserts.h"

namespace impeller {
namespace testing {

TEST(RectTest, RectOriginSizeGetters) {
  {
    Rect r = Rect::MakeOriginSize({10, 20}, {50, 40});
    EXPECT_EQ(r.GetOrigin(), Point(10, 20));
    EXPECT_EQ(r.GetSize(), Size(50, 40));
  }

  {
    Rect r = Rect::MakeLTRB(10, 20, 50, 40);
    EXPECT_EQ(r.GetOrigin(), Point(10, 20));
    EXPECT_EQ(r.GetSize(), Size(40, 20));
  }
}

TEST(RectTest, RectMakeSize) {
  {
    Size s(100, 200);
    Rect r = Rect::MakeSize(s);
    Rect expected = Rect::MakeLTRB(0, 0, 100, 200);
    ASSERT_RECT_NEAR(r, expected);
  }

  {
    ISize s(100, 200);
    Rect r = Rect::MakeSize(s);
    Rect expected = Rect::MakeLTRB(0, 0, 100, 200);
    ASSERT_RECT_NEAR(r, expected);
  }

  {
    Size s(100, 200);
    IRect r = IRect::MakeSize(s);
    IRect expected = IRect::MakeLTRB(0, 0, 100, 200);
    EXPECT_EQ(r, expected);
  }

  {
    ISize s(100, 200);
    IRect r = IRect::MakeSize(s);
    IRect expected = IRect::MakeLTRB(0, 0, 100, 200);
    EXPECT_EQ(r, expected);
  }
}

TEST(RectTest, NormalizePoint) {
  // Tests finite rects including:
  // - points at the corners, inside, and outside
  // - rects that are non-empty or empty through either zero or
  //   negative width and/or height
  // - all combinations of integer and scalar rects and points.

  // Tests one rectangle in all 4 combinations of integer and scalar data
  // and also against NaN point values
  auto test_one = [](int64_t l, int64_t t, int64_t r, int64_t b,  //
                     const std::string& rect_desc,                //
                     int64_t px, int64_t py,                      //
                     const std::string& pt_desc,                  //
                     Point expected) {
    // Scalar point inside Scalar rect
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(Point(px, py)),
              expected)
        << "Point(" << pt_desc << ") in " << rect_desc << " Rect";

    // Scalar point inside Integer rect
    EXPECT_EQ(IRect::MakeLTRB(l, t, r, b).NormalizePoint(Point(px, py)),
              expected)
        << "Point(" << pt_desc << ") in " << rect_desc << " IRect";

    // Integer point inside Scalar rect
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(IPoint(px, py)),
              expected)
        << "IPoint(" << pt_desc << ") in " << rect_desc << " Rect";

    // Integer point inside Integer rect
    EXPECT_EQ(IRect::MakeLTRB(l, t, r, b).NormalizePoint(IPoint(px, py)),
              expected)
        << "IPoint(" << pt_desc << ") in " << rect_desc << " IRect";

    // Now test nan handling by substituting a nan for the X and/or Y of
    // the point.
    auto nan = std::numeric_limits<Scalar>::quiet_NaN();
    auto nan_x = Point(nan, py);
    auto nan_y = Point(px, nan);
    auto nan_p = Point(nan, nan);
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(nan_x), Point())
        << "Point(NaN x) in " << rect_desc << " Rect";
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(nan_y), Point())
        << "Point(NaN y) in " << rect_desc << " Rect";
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(nan_p), Point())
        << "Point(NaN x&y) in " << rect_desc << " Rect";
    EXPECT_EQ(IRect::MakeLTRB(l, t, r, b).NormalizePoint(nan_x), Point())
        << "Point(NaN x) in " << rect_desc << " Rect";
    EXPECT_EQ(IRect::MakeLTRB(l, t, r, b).NormalizePoint(nan_y), Point())
        << "Point(NaN y) in " << rect_desc << " Rect";
    EXPECT_EQ(IRect::MakeLTRB(l, t, r, b).NormalizePoint(nan_p), Point())
        << "Point(NaN x&y) in " << rect_desc << " Rect";
  };

  // Tests the rectangle as specified, which is assumed to be non-empty
  // and then tests it in various states of emptiness in both dimensions
  // by reversing left/right and top/bottom or using the same value for
  // both.
  auto test = [&test_one](int64_t l, int64_t t, int64_t r, int64_t b,  //
                          int64_t px, int64_t py,                      //
                          const std::string& pt_desc, Point expected) {
    // clang-format off
    //                         rect                    point   expected
    //         rect         description     point      desc     result
    test_one(l, t, r, b,    "non-empty",   px, py,   pt_desc,  expected);
    test_one(l, t, l, b,    "LR empty",    px, py,   pt_desc,  Point());
    test_one(r, t, l, b,   "LR reversed",  px, py,   pt_desc,  Point());
    test_one(l, t, r, t,    "TB empty",    px, py,   pt_desc,  Point());
    test_one(l, b, r, t,   "TB reversed",  px, py,   pt_desc,  Point());
    test_one(l, t, l, t,    "all empty",   px, py,   pt_desc,  Point());
    test_one(r, b, l, t,  "all reversed",  px, py,   pt_desc,  Point());
    // clang-format on
  };

  // clang-format off
  //        rect             point        point          expected
  //  [ l    t    r    b ] [ px   py ]  description       result
  test(100, 100, 200, 200,  100, 100,      "UL",       Point(0, 0));
  test(100, 100, 200, 200,  200, 100,      "UR",       Point(1, 0));
  test(100, 100, 200, 200,  200, 200,      "LR",       Point(1, 1));
  test(100, 100, 200, 200,  100, 200,      "LL",       Point(0, 1));
  test(100, 100, 200, 200,  150, 150,    "Center",     Point(0.5, 0.5));
  test(100, 100, 200, 200,    0,   0,   "outside UL",  Point(-1, -1));
  test(100, 100, 200, 200,  300,   0,   "outside UR",  Point(2, -1));
  test(100, 100, 200, 200,  300, 300,   "outside LR",  Point(2, 2));
  test(100, 100, 200, 200,    0, 300,   "outside LL",  Point(-1, 2));
  // clang-format on

  // We can't test the true min and max due to overflow of the xywh
  // internal representation, but we can test with half their values.
  // When TRect is converted to ltrb notation, we can relax this
  // restriction.
  int64_t min_int = std::numeric_limits<int64_t>::min() / 2;
  int64_t max_int = std::numeric_limits<int64_t>::max() / 2;
  test(min_int, 100, max_int, 200, 0, 150, "max int center", Point(0.5, 0.5));
}

TEST(RectTest, NormalizePointToNonFiniteRects) {
  // Tests non-finite Scalar rects including:
  // - points at the corners, inside, and outside
  // - rects that are non-empty or empty through either zero or
  //   negative width and/or height

  // Tests one rectangle against supplied point values and NaN replacements.
  auto test = [](Scalar l, Scalar t, Scalar r, Scalar b,  //
                 Scalar px, Scalar py,                    //
                 const std::string& pt_desc,              //
                 Point expected) {
    auto nan = std::numeric_limits<Scalar>::quiet_NaN();
    auto inf = std::numeric_limits<Scalar>::infinity();

    // Scalar point inside Scalar rect
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(Point(px, py)),
              expected)
        << "Point(" << pt_desc << ") in Rect";

    // Scalar point inside Scalar rect with NaN left
    EXPECT_EQ(Rect::MakeLTRB(nan, t, r, b).NormalizePoint(Point(px, py)),
              Point())
        << "Point(" << pt_desc << ") in Rect NaN Left";

    // Scalar point inside Scalar rect with NaN top
    EXPECT_EQ(Rect::MakeLTRB(l, nan, r, b).NormalizePoint(Point(px, py)),
              Point())
        << "Point(" << pt_desc << ") in Rect NaN Top";

    // Scalar point inside Scalar rect with NaN right
    EXPECT_EQ(Rect::MakeLTRB(l, t, nan, b).NormalizePoint(Point(px, py)),
              Point())
        << "Point(" << pt_desc << ") in Rect NaN Left";

    // Scalar point inside Scalar rect with NaN bottom
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, nan).NormalizePoint(Point(px, py)),
              Point())
        << "Point(" << pt_desc << ") in Rect NaN Top";

    // Scalar point inside Scalar rect with infinite left
    EXPECT_EQ(Rect::MakeLTRB(-inf, t, r, b).NormalizePoint(Point(px, py)),
              Point())
        << "Point(" << pt_desc << ") in Rect -Inf Left";

    // Scalar point inside Scalar rect with infinite top
    EXPECT_EQ(Rect::MakeLTRB(l, -inf, r, b).NormalizePoint(Point(px, py)),
              Point())
        << "Point(" << pt_desc << ") in Rect -Inf Top";

    // Scalar point inside Scalar rect with infinite right
    EXPECT_EQ(Rect::MakeLTRB(l, t, inf, b).NormalizePoint(Point(px, py)),
              Point(0, expected.y))
        << "Point(" << pt_desc << ") in Rect Inf Right";

    // Scalar point inside Scalar rect with infinite bottom
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, inf).NormalizePoint(Point(px, py)),
              Point(expected.x, 0))
        << "Point(" << pt_desc << ") in Rect Inf Bottom";

    // Now test NaN handling by substituting a NaN for the X and/or Y of
    // the point.
    auto nan_x = Point(nan, py);
    auto nan_y = Point(px, nan);
    auto nan_p = Point(nan, nan);
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(nan_x), Point())
        << "Point(NaN x) in Rect";
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(nan_y), Point())
        << "Point(NaN y) in Rect";
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(nan_p), Point())
        << "Point(NaN x&y) in Rect";

    // Now test +/- infinity handling by substituting an infinity for the
    // X and/or Y of the point.
    auto inf_x = Point(inf, py);
    auto inf_y = Point(px, inf);
    auto inf_p = Point(inf, inf);
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(inf_x), Point())
        << "Point(Infinite x) in Rect";
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(inf_y), Point())
        << "Point(Infinite y) in Rect";
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(inf_p), Point())
        << "Point(Infinite x&y) in Rect";
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(-inf_x), Point())
        << "Point(-Infinite x) in Rect";
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(-inf_y), Point())
        << "Point(-Infinite y) in Rect";
    EXPECT_EQ(Rect::MakeLTRB(l, t, r, b).NormalizePoint(-inf_p), Point())
        << "Point(-Infinite x&y) in Rect";
  };

  // clang-format off
  //        rect             point         point         expected
  //  [ l    t    r    b ] [ px   py ]   description       result
  test(100, 100, 200, 200,  100, 100,       "UL",       Point(0, 0));
  test(100, 100, 200, 200,  200, 100,       "UR",       Point(1, 0));
  test(100, 100, 200, 200,  200, 200,       "LR",       Point(1, 1));
  test(100, 100, 200, 200,  100, 200,       "LL",       Point(0, 1));
  test(100, 100, 200, 200,  150, 150,     "Center",     Point(0.5, 0.5));
  test(100, 100, 200, 200,    0,   0,   "outside UL",   Point(-1, -1));
  test(100, 100, 200, 200,  300,   0,   "outside UR",   Point(2, -1));
  test(100, 100, 200, 200,  300, 300,   "outside LR",   Point(2, 2));
  test(100, 100, 200, 200,    0, 300,   "outside LL",   Point(-1, 2));
  // clang-format on
}

}  // namespace testing
}  // namespace impeller
