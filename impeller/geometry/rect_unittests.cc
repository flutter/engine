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

// Performs an EXPECT_EQ on the operation of Rect::NormalizePoint and
// outputs the raw data for the test case on failure to help with diagnosis
#define TEST_NORMALIZE_POINT(RT, PT, l, t, r, b, px, py, expected)            \
  EXPECT_EQ(RT::MakeLTRB(l, t, r, b).NormalizePoint(PT(px, py)), expected)    \
      << #RT "::MakeLTRB(" << l << ", " << t << ", " << r << ", " << b << ")" \
      << ".NormalizePoint(" << #PT "(" << px << ", " << py << "))"            \
      << std::endl

TEST(RectTest, NormalizePoint) {
  // Tests finite rects including:
  // - points at the corners, inside, and outside
  // - rects that are non-empty or empty through either zero or
  //   negative width and/or height
  // - all combinations of integer and scalar rects and points.

  // Tests one rectangle against one point in all 4 combinations of integer
  // and scalar data types and with NaN and infinity point values
  auto test_one = [](int64_t l, int64_t t, int64_t r, int64_t b,  //
                     int64_t px, int64_t py,                      //
                     Point expected) {
    // First test with all combinations of [I]Rect and [I]Point
    // clang-format off
    TEST_NORMALIZE_POINT( Rect,  Point, l, t, r, b, px, py, expected);
    TEST_NORMALIZE_POINT(IRect,  Point, l, t, r, b, px, py, expected);
    TEST_NORMALIZE_POINT( Rect, IPoint, l, t, r, b, px, py, expected);
    TEST_NORMALIZE_POINT(IRect, IPoint, l, t, r, b, px, py, expected);
    // clang-format on

    // Now test nan handling by substituting a nan for the X and/or Y of
    // the point.
    auto nan = std::numeric_limits<Scalar>::quiet_NaN();
    // Test with Rect and IRect, but we have to use Point to hold the NaN
    // clang-format off
    TEST_NORMALIZE_POINT( Rect, Point, l, t, r, b, nan,  py, Point());
    TEST_NORMALIZE_POINT( Rect, Point, l, t, r, b,  px, nan, Point());
    TEST_NORMALIZE_POINT( Rect, Point, l, t, r, b, nan, nan, Point());
    TEST_NORMALIZE_POINT(IRect, Point, l, t, r, b, nan,  py, Point());
    TEST_NORMALIZE_POINT(IRect, Point, l, t, r, b,  px, nan, Point());
    TEST_NORMALIZE_POINT(IRect, Point, l, t, r, b, nan, nan, Point());
    // clang-format on

    // Now test +/- infinity handling by substituting an infinity for the
    // X and/or Y of the point.
    auto inf = std::numeric_limits<Scalar>::infinity();

    // Test with Rect, but we have to use Point to hold the infinity
    // clang-format off
    TEST_NORMALIZE_POINT( Rect, Point, l, t, r, b,  inf,   py, Point());
    TEST_NORMALIZE_POINT( Rect, Point, l, t, r, b,   px,  inf, Point());
    TEST_NORMALIZE_POINT( Rect, Point, l, t, r, b,  inf,  inf, Point());
    TEST_NORMALIZE_POINT( Rect, Point, l, t, r, b, -inf,   py, Point());
    TEST_NORMALIZE_POINT( Rect, Point, l, t, r, b,   px, -inf, Point());
    TEST_NORMALIZE_POINT( Rect, Point, l, t, r, b, -inf, -inf, Point());
    // clang-format on

    // Test with IRect, but we have to use Point to hold the infinity
    // clang-format off
    TEST_NORMALIZE_POINT(IRect, Point, l, t, r, b,  inf,   py, Point());
    TEST_NORMALIZE_POINT(IRect, Point, l, t, r, b,   px,  inf, Point());
    TEST_NORMALIZE_POINT(IRect, Point, l, t, r, b,  inf,  inf, Point());
    TEST_NORMALIZE_POINT(IRect, Point, l, t, r, b, -inf,   py, Point());
    TEST_NORMALIZE_POINT(IRect, Point, l, t, r, b,   px, -inf, Point());
    TEST_NORMALIZE_POINT(IRect, Point, l, t, r, b, -inf, -inf, Point());
    // clang-format on
  };

  // Tests the rectangle as specified, which is assumed to be non-empty
  // and then tests it in various states of emptiness in both dimensions
  // by reversing left/right and top/bottom or using the same value for
  // both.
  auto test = [&test_one](int64_t l, int64_t t, int64_t r, int64_t b,  //
                          int64_t px, int64_t py,                      //
                          Point expected) {
    // clang-format off
    //                             expected
    //         rect        point    result
    test_one(l, t, r, b,  px, py,  expected);  // normal
    test_one(l, t, l, b,  px, py,  Point());   // empty horizontally
    test_one(r, t, l, b,  px, py,  Point());   // reversed horizontally
    test_one(l, t, r, t,  px, py,  Point());   // empty vertically
    test_one(l, b, r, t,  px, py,  Point());   // reversed vertically
    test_one(l, t, l, t,  px, py,  Point());   // empty in both axes
    test_one(r, b, l, t,  px, py,  Point());   // reversed in both axes
    // clang-format on
  };

  // clang-format off
  //        rect             point       expected
  //  [ l    t    r    b ] [ px   py ]    result
  test(100, 100, 200, 200,  100, 100,   Point(0, 0));      // Upper Left  (UL)
  test(100, 100, 200, 200,  200, 100,   Point(1, 0));      // Upper Right (UR)
  test(100, 100, 200, 200,  200, 200,   Point(1, 1));      // Lower Right (LR)
  test(100, 100, 200, 200,  100, 200,   Point(0, 1));      // Lower Left  (LL)
  test(100, 100, 200, 200,  150, 150,   Point(0.5, 0.5));  // Center
  test(100, 100, 200, 200,    0,   0,   Point(-1, -1));    // Outside UL
  test(100, 100, 200, 200,  300,   0,   Point(2, -1));     // Outside UR
  test(100, 100, 200, 200,  300, 300,   Point(2, 2));      // Outside LR
  test(100, 100, 200, 200,    0, 300,   Point(-1, 2));     // Outside LL
  // clang-format on

  // We can't test the true min and max 64-bit values due to overflow of
  // the xywh internal representation, but we can test with half their
  // values.
  //
  // When TRect is converted to ltrb notation, we can relax this
  // restriction.
  int64_t min_int = std::numeric_limits<int64_t>::min() / 2;
  int64_t max_int = std::numeric_limits<int64_t>::max() / 2;

  // clang-format off
  test(min_int,     100, max_int,     200,   0, 150, Point(0.5, 0.5));
  test(    100, min_int,     200, max_int, 150,   0, Point(0.5, 0.5));
  test(min_int, min_int, max_int, max_int, 150, 150, Point(0.5, 0.5));
  // clang-format on
}

TEST(RectTest, NormalizePointToNonFiniteRects) {
  // Tests non-finite Scalar rects including:
  // - points at the corners, inside, and outside
  // - rects that are non-empty or empty through either zero or
  //   negative width and/or height

  // Tests one rectangle against supplied point values and NaN replacements.
  auto test = [](Scalar l, Scalar t, Scalar r, Scalar b,  //
                 Scalar px, Scalar py,                    //
                 Point expected) {
    auto nan = std::numeric_limits<Scalar>::quiet_NaN();
    auto inf = std::numeric_limits<Scalar>::infinity();

    TEST_NORMALIZE_POINT(Rect, Point, l, t, r, b, px, py, expected);

    // Now retest with NaN replacing each rect parameter - producing (0, 0)
    TEST_NORMALIZE_POINT(Rect, Point, nan, t, r, b, px, py, Point());
    TEST_NORMALIZE_POINT(Rect, Point, l, nan, r, b, px, py, Point());
    TEST_NORMALIZE_POINT(Rect, Point, l, t, nan, b, px, py, Point());
    TEST_NORMALIZE_POINT(Rect, Point, l, t, r, nan, px, py, Point());

    // Now retest with -inf replacing the left/top values - producing (0, 0)
    TEST_NORMALIZE_POINT(Rect, Point, -inf, t, r, b, px, py, Point());
    TEST_NORMALIZE_POINT(Rect, Point, l, -inf, r, b, px, py, Point());

    // Now retest with +inf replacing the right/bottom values which
    // produces a valid result, but the associated normalized coordinate
    // will always be 0.
    TEST_NORMALIZE_POINT(Rect, Point, l, t, inf, b, px, py,
                         Point(0, expected.y));
    TEST_NORMALIZE_POINT(Rect, Point, l, t, r, inf, px, py,
                         Point(expected.x, 0));
  };

  // clang-format off
  //          rect             point       expected
  //  [ l    t    r    b ]  [ px   py ]     result
  test(100, 100, 200, 200,   100, 100,   Point(0, 0));      // Upper Left  (UL)
  test(100, 100, 200, 200,   200, 100,   Point(1, 0));      // Upper Right (UR)
  test(100, 100, 200, 200,   200, 200,   Point(1, 1));      // Lower Right (LR)
  test(100, 100, 200, 200,   100, 200,   Point(0, 1));      // Lower Left  (LL)
  test(100, 100, 200, 200,   150, 150,   Point(0.5, 0.5));  // Center
  test(100, 100, 200, 200,     0,   0,   Point(-1, -1));    // Outside UL
  test(100, 100, 200, 200,   300,   0,   Point(2, -1));     // Outside UR
  test(100, 100, 200, 200,   300, 300,   Point(2, 2));      // Outside LR
  test(100, 100, 200, 200,     0, 300,   Point(-1, 2));     // Outside LL
  // clang-format on
}

#undef TEST_NORMALIZE_POINT

}  // namespace testing
}  // namespace impeller
