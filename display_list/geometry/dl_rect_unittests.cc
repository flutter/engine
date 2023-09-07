// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_rect.h"
#include "fml/logging.h"
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
  ASSERT_TRUE(rect.IsEmpty());
  ASSERT_TRUE(rect.IsFinite());
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
  ASSERT_TRUE(rect.IsEmpty());
  // ASSERT_TRUE(rect.IsFinite());  // should fail to compile
}

TEST(DlRectTest, FRectDefaultConstructor) {
  DlFRect rect = DlFRect();

  ASSERT_EQ(rect.left(), 0.0f);
  ASSERT_EQ(rect.top(), 0.0f);
  ASSERT_EQ(rect.right(), 0.0f);
  ASSERT_EQ(rect.bottom(), 0.0f);
  ASSERT_EQ(rect.x(), 0.0f);
  ASSERT_EQ(rect.y(), 0.0f);
  ASSERT_EQ(rect.width(), 0.0f);
  ASSERT_EQ(rect.height(), 0.0f);
  ASSERT_TRUE(rect.IsEmpty());
  ASSERT_TRUE(rect.IsFinite());
}

TEST(DlRectTest, IRectDefaultConstructor) {
  DlIRect rect = DlIRect();

  ASSERT_EQ(rect.left(), 0);
  ASSERT_EQ(rect.top(), 0);
  ASSERT_EQ(rect.right(), 0);
  ASSERT_EQ(rect.bottom(), 0);
  ASSERT_EQ(rect.x(), 0);
  ASSERT_EQ(rect.y(), 0);
  ASSERT_EQ(rect.width(), 0u);
  ASSERT_EQ(rect.height(), 0u);
  ASSERT_TRUE(rect.IsEmpty());
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
  ASSERT_FALSE(rect.IsEmpty());
  ASSERT_TRUE(rect.IsFinite());
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
  ASSERT_FALSE(rect.IsEmpty());
}

TEST(DlRectTest, FRectRoundingEmpty) {
  DlFRect rect;

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

  ASSERT_EQ(DlIRect::MakeRoundedOut(rect), DlIRect::MakeLTRB(5, 10, 21, 26));
  ASSERT_EQ(DlIRect::MakeRoundedIn(rect), DlIRect::MakeLTRB(6, 11, 20, 25));
  ASSERT_EQ(DlIRect::MakeRounded(rect), DlIRect::MakeLTRB(5, 10, 21, 25));

  ASSERT_EQ(rect.RoundedOut(), DlFRect::MakeLTRB(5.0f, 10.0f, 21.0f, 26.0f));
  ASSERT_EQ(rect.RoundedIn(), DlFRect::MakeLTRB(6.0f, 11.0f, 20.0f, 25.0f));
  ASSERT_EQ(rect.Rounded(), DlFRect::MakeLTRB(5.0f, 10.0f, 21.0f, 25.0f));
}

TEST(DlRectTest, FRectRoundToIRectHuge) {
  auto test = [](int corners) {
    ASSERT_TRUE(corners >= 0 && corners <= 0xf);
    DlScalar l, t, r, b;
    DlInt il, it, ir, ib;
    l = il = 50;
    t = it = 50;
    r = ir = 80;
    b = ib = 80;
    if ((corners & (1 << 0)) != 0) {
      l = -1E20;
      il = std::numeric_limits<DlInt>::min();
    }
    if ((corners & (1 << 1)) != 0) {
      t = -1E20;
      it = std::numeric_limits<DlInt>::min();
    }
    if ((corners & (1 << 2)) != 0) {
      r = +1E20;
      ir = std::numeric_limits<DlInt>::max();
    }
    if ((corners & (1 << 3)) != 0) {
      b = +1E20;
      ib = std::numeric_limits<DlInt>::max();
    }

    DlFRect rect = DlFRect::MakeLTRB(l, t, r, b);
    DlIRect irect = DlIRect::MakeRounded(rect);
    EXPECT_EQ(irect.left(), il) << corners;
    EXPECT_EQ(irect.top(), it) << corners;
    EXPECT_EQ(irect.right(), ir) << corners;
    EXPECT_EQ(irect.bottom(), ib) << corners;
  };

  for (int corners = 0; corners <= 15; corners++) {
    test(corners);
  }
}

TEST(DlRectTest, FRectRoundOutToIRectHuge) {
  auto test = [](int corners) {
    ASSERT_TRUE(corners >= 0 && corners <= 0xf);
    DlScalar l, t, r, b;
    DlInt il, it, ir, ib;
    l = il = 50;
    t = it = 50;
    r = ir = 80;
    b = ib = 80;
    if ((corners & (1 << 0)) != 0) {
      l = -1E20;
      il = std::numeric_limits<DlInt>::min();
    }
    if ((corners & (1 << 1)) != 0) {
      t = -1E20;
      it = std::numeric_limits<DlInt>::min();
    }
    if ((corners & (1 << 2)) != 0) {
      r = +1E20;
      ir = std::numeric_limits<DlInt>::max();
    }
    if ((corners & (1 << 3)) != 0) {
      b = +1E20;
      ib = std::numeric_limits<DlInt>::max();
    }

    DlFRect rect = DlFRect::MakeLTRB(l, t, r, b);
    DlIRect irect = DlIRect::MakeRoundedOut(rect);
    EXPECT_EQ(irect.left(), il) << corners;
    EXPECT_EQ(irect.top(), it) << corners;
    EXPECT_EQ(irect.right(), ir) << corners;
    EXPECT_EQ(irect.bottom(), ib) << corners;
  };

  for (int corners = 0; corners <= 15; corners++) {
    test(corners);
  }
}

TEST(DlRectTest, FRectRoundInToIRectHuge) {
  auto test = [](int corners) {
    ASSERT_TRUE(corners >= 0 && corners <= 0xf);
    DlScalar l, t, r, b;
    DlInt il, it, ir, ib;
    l = il = 50;
    t = it = 50;
    r = ir = 80;
    b = ib = 80;
    if ((corners & (1 << 0)) != 0) {
      l = -1E20;
      il = std::numeric_limits<DlInt>::min();
    }
    if ((corners & (1 << 1)) != 0) {
      t = -1E20;
      it = std::numeric_limits<DlInt>::min();
    }
    if ((corners & (1 << 2)) != 0) {
      r = +1E20;
      ir = std::numeric_limits<DlInt>::max();
    }
    if ((corners & (1 << 3)) != 0) {
      b = +1E20;
      ib = std::numeric_limits<DlInt>::max();
    }

    DlFRect rect = DlFRect::MakeLTRB(l, t, r, b);
    DlIRect irect = DlIRect::MakeRoundedIn(rect);
    EXPECT_EQ(irect.left(), il) << corners;
    EXPECT_EQ(irect.top(), it) << corners;
    EXPECT_EQ(irect.right(), ir) << corners;
    EXPECT_EQ(irect.bottom(), ib) << corners;
  };

  for (int corners = 0; corners <= 15; corners++) {
    test(corners);
  }
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
  ASSERT_FALSE(copy.IsEmpty());
  ASSERT_TRUE(copy.IsFinite());
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
  ASSERT_FALSE(copy.IsEmpty());
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

  test(70, 70, 80, 80, "Inside");

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

  test(70, 70, 80, 80, "Inside");

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

  test(70, 70, 80, 80, "Inside");

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

TEST(DlRectTest, IRectCutOut) {
  DlIRect cull_rect = DlIRect::MakeLTRB(20, 20, 40, 40);
  DlIRect empty_rect;

  auto check_empty_flips = [&cull_rect, &empty_rect](const DlIRect& diff_rect,
                                                     const std::string& label) {
    ASSERT_FALSE(diff_rect.IsEmpty());
    ASSERT_FALSE(cull_rect.IsEmpty());

    // unflipped cull_rect vs flipped(empty) diff_rect
    // == cull_rect
    ASSERT_TRUE(cull_rect.CutOut(flip_lr(diff_rect)).has_value());
    ASSERT_EQ(cull_rect.CutOut(flip_lr(diff_rect)), cull_rect);
    ASSERT_TRUE(cull_rect.CutOut(flip_tb(diff_rect)).has_value());
    ASSERT_EQ(cull_rect.CutOut(flip_tb(diff_rect)), cull_rect);
    ASSERT_TRUE(cull_rect.CutOut(flip_lrtb(diff_rect)).has_value());
    ASSERT_EQ(cull_rect.CutOut(flip_lrtb(diff_rect)), cull_rect);

    // flipped(empty) cull_rect vs flipped(empty) diff_rect
    // == empty
    ASSERT_FALSE(flip_lr(cull_rect).CutOut(diff_rect).has_value());
    ASSERT_EQ(flip_lr(cull_rect).CutOutOrEmpty(diff_rect), empty_rect);
    ASSERT_FALSE(flip_tb(cull_rect).CutOut(diff_rect).has_value());
    ASSERT_EQ(flip_tb(cull_rect).CutOutOrEmpty(diff_rect), empty_rect);
    ASSERT_FALSE(flip_lrtb(cull_rect).CutOut(diff_rect).has_value());
    ASSERT_EQ(flip_lrtb(cull_rect).CutOutOrEmpty(diff_rect), empty_rect);

    // flipped(empty) cull_rect vs unflipped diff_rect
    // == empty
    ASSERT_FALSE(flip_lr(cull_rect).CutOut(flip_lr(diff_rect)).has_value());
    ASSERT_EQ(flip_lr(cull_rect).CutOutOrEmpty(flip_lr(diff_rect)), empty_rect);
    ASSERT_FALSE(flip_tb(cull_rect).CutOut(flip_tb(diff_rect)).has_value());
    ASSERT_EQ(flip_tb(cull_rect).CutOutOrEmpty(flip_tb(diff_rect)), empty_rect);
    ASSERT_FALSE(flip_lrtb(cull_rect).CutOut(flip_lrtb(diff_rect)).has_value());
    ASSERT_EQ(flip_lrtb(cull_rect).CutOutOrEmpty(flip_lrtb(diff_rect)),
              empty_rect);
  };

  auto non_reducing = [&cull_rect, &check_empty_flips](
                          const DlIRect& diff_rect, const std::string& label) {
    ASSERT_EQ(cull_rect.CutOut(diff_rect), cull_rect) << label;
    check_empty_flips(diff_rect, label);
  };

  auto reducing = [&cull_rect, &check_empty_flips](const DlIRect& diff_rect,
                                                   const DlIRect& result_rect,
                                                   const std::string& label) {
    ASSERT_TRUE(!result_rect.IsEmpty());
    ASSERT_EQ(cull_rect.CutOut(diff_rect), result_rect) << label;
    check_empty_flips(diff_rect, label);
  };

  auto emptying = [&cull_rect, &empty_rect, &check_empty_flips](
                      const DlIRect& diff_rect, const std::string& label) {
    ASSERT_FALSE(cull_rect.CutOut(diff_rect).has_value()) << label;
    ASSERT_EQ(cull_rect.CutOutOrEmpty(diff_rect), empty_rect) << label;
    check_empty_flips(diff_rect, label);
  };

  // Skim the corners and edge
  non_reducing(DlIRect::MakeLTRB(10, 10, 20, 20), "outside UL corner");
  non_reducing(DlIRect::MakeLTRB(20, 10, 40, 20), "Above");
  non_reducing(DlIRect::MakeLTRB(40, 10, 50, 20), "outside UR corner");
  non_reducing(DlIRect::MakeLTRB(40, 20, 50, 40), "Right");
  non_reducing(DlIRect::MakeLTRB(40, 40, 50, 50), "outside LR corner");
  non_reducing(DlIRect::MakeLTRB(20, 40, 40, 50), "Below");
  non_reducing(DlIRect::MakeLTRB(10, 40, 20, 50), "outside LR corner");
  non_reducing(DlIRect::MakeLTRB(10, 20, 20, 40), "Left");

  // Overlap corners
  non_reducing(DlIRect::MakeLTRB(15, 15, 25, 25), "covering UL corner");
  non_reducing(DlIRect::MakeLTRB(35, 15, 45, 25), "covering UR corner");
  non_reducing(DlIRect::MakeLTRB(35, 35, 45, 45), "covering LR corner");
  non_reducing(DlIRect::MakeLTRB(15, 35, 25, 45), "covering LL corner");

  // Overlap edges, but not across an entire side
  non_reducing(DlIRect::MakeLTRB(20, 15, 39, 25), "Top edge left-biased");
  non_reducing(DlIRect::MakeLTRB(21, 15, 40, 25), "Top edge, right biased");
  non_reducing(DlIRect::MakeLTRB(35, 20, 45, 39), "Right edge, top-biased");
  non_reducing(DlIRect::MakeLTRB(35, 21, 45, 40), "Right edge, bottom-biased");
  non_reducing(DlIRect::MakeLTRB(20, 35, 39, 45), "Bottom edge, left-biased");
  non_reducing(DlIRect::MakeLTRB(21, 35, 40, 45), "Bottom edge, right-biased");
  non_reducing(DlIRect::MakeLTRB(15, 20, 25, 39), "Left edge, top-biased");
  non_reducing(DlIRect::MakeLTRB(15, 21, 25, 40), "Left edge, bottom-biased");

  // Slice all the way through the middle
  non_reducing(DlIRect::MakeLTRB(25, 15, 35, 45), "Vertical interior slice");
  non_reducing(DlIRect::MakeLTRB(15, 25, 45, 35), "Horizontal interior slice");

  // Slice off each edge
  reducing(DlIRect::MakeLTRB(20, 15, 40, 25),  //
           DlIRect::MakeLTRB(20, 25, 40, 40),  //
           "Slice off top");
  reducing(DlIRect::MakeLTRB(35, 20, 45, 40),  //
           DlIRect::MakeLTRB(20, 20, 35, 40),  //
           "Slice off right");
  reducing(DlIRect::MakeLTRB(20, 35, 40, 45),  //
           DlIRect::MakeLTRB(20, 20, 40, 35),  //
           "Slice off bottom");
  reducing(DlIRect::MakeLTRB(15, 20, 25, 40),  //
           DlIRect::MakeLTRB(25, 20, 40, 40),  //
           "Slice off left");

  // cull rect contains diff rect
  non_reducing(DlIRect::MakeLTRB(21, 21, 39, 39), "Contained, non-covering");

  // cull rect equals diff rect
  emptying(cull_rect, "Perfectly covering");

  // diff rect contains cull rect
  emptying(DlIRect::MakeLTRB(15, 15, 45, 45), "Smothering");
}

static constexpr inline DlFRect swap_nan(const DlFRect& rect, int index) {
  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  FML_DCHECK(index >= 0 && index < 4);
  DlScalar l = (index == 0) ? nan : rect.left();
  DlScalar t = (index == 1) ? nan : rect.top();
  DlScalar r = (index == 2) ? nan : rect.right();
  DlScalar b = (index == 3) ? nan : rect.bottom();
  return DlFRect::MakeLTRB(l, t, r, b);
}

TEST(DlRectTest, FRectCutOut) {
  DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 40, 40);
  DlFRect empty_rect;

  auto check_nans = [&cull_rect, &empty_rect](const DlFRect& diff_rect,
                                              const std::string& label) {
    ASSERT_TRUE(cull_rect.IsFinite()) << label;
    ASSERT_TRUE(diff_rect.IsFinite()) << label;

    for (int i = 0; i < 4; i++) {
      // NaN in cull_rect produces empty
      ASSERT_FALSE(swap_nan(cull_rect, i).CutOut(diff_rect).has_value())
          << label << ", index " << i;
      ASSERT_EQ(swap_nan(cull_rect, i).CutOutOrEmpty(diff_rect), empty_rect)
          << label << ", index " << i;

      // NaN in diff_rect is nop
      ASSERT_TRUE(cull_rect.CutOut(swap_nan(diff_rect, i)).has_value())
          << label << ", index " << i;
      ASSERT_EQ(cull_rect.CutOutOrEmpty(swap_nan(diff_rect, i)), cull_rect)
          << label << ", index " << i;

      for (int j = 0; j < 4; j++) {
        // NaN in both is also empty
        ASSERT_FALSE(
            swap_nan(cull_rect, i).CutOut(swap_nan(diff_rect, j)).has_value())
            << label << ", indices " << i << ", " << j;
        ASSERT_EQ(swap_nan(cull_rect, i).CutOutOrEmpty(swap_nan(diff_rect, j)),
                  empty_rect)
            << label << ", indices " << i << ", " << j;
      }
    }
  };

  auto check_empty_flips = [&cull_rect, &empty_rect](const DlFRect& diff_rect,
                                                     const std::string& label) {
    ASSERT_FALSE(cull_rect.IsEmpty()) << label;
    ASSERT_FALSE(diff_rect.IsEmpty()) << label;

    // unflipped cull_rect vs flipped(empty) diff_rect
    // == cull_rect
    ASSERT_TRUE(cull_rect.CutOut(flip_lr(diff_rect)).has_value()) << label;
    ASSERT_EQ(cull_rect.CutOut(flip_lr(diff_rect)), cull_rect) << label;
    ASSERT_TRUE(cull_rect.CutOut(flip_tb(diff_rect)).has_value()) << label;
    ASSERT_EQ(cull_rect.CutOut(flip_tb(diff_rect)), cull_rect) << label;
    ASSERT_TRUE(cull_rect.CutOut(flip_lrtb(diff_rect)).has_value()) << label;
    ASSERT_EQ(cull_rect.CutOut(flip_lrtb(diff_rect)), cull_rect) << label;

    // flipped(empty) cull_rect vs flipped(empty) diff_rect
    // == empty
    ASSERT_FALSE(flip_lr(cull_rect).CutOut(diff_rect).has_value()) << label;
    ASSERT_EQ(flip_lr(cull_rect).CutOutOrEmpty(diff_rect), empty_rect) << label;
    ASSERT_FALSE(flip_tb(cull_rect).CutOut(diff_rect).has_value()) << label;
    ASSERT_EQ(flip_tb(cull_rect).CutOutOrEmpty(diff_rect), empty_rect) << label;
    ASSERT_FALSE(flip_lrtb(cull_rect).CutOut(diff_rect).has_value()) << label;
    ASSERT_EQ(flip_lrtb(cull_rect).CutOutOrEmpty(diff_rect), empty_rect)
        << label;

    // flipped(empty) cull_rect vs unflipped diff_rect
    // == empty
    ASSERT_FALSE(flip_lr(cull_rect).CutOut(flip_lr(diff_rect)).has_value())
        << label;
    ASSERT_EQ(flip_lr(cull_rect).CutOutOrEmpty(flip_lr(diff_rect)), empty_rect)
        << label;
    ASSERT_FALSE(flip_tb(cull_rect).CutOut(flip_tb(diff_rect)).has_value())
        << label;
    ASSERT_EQ(flip_tb(cull_rect).CutOutOrEmpty(flip_tb(diff_rect)), empty_rect)
        << label;
    ASSERT_FALSE(flip_lrtb(cull_rect).CutOut(flip_lrtb(diff_rect)).has_value())
        << label;
    ASSERT_EQ(flip_lrtb(cull_rect).CutOutOrEmpty(flip_lrtb(diff_rect)),
              empty_rect)
        << label;
  };

  auto non_reducing = [&cull_rect, &check_empty_flips, &check_nans](
                          const DlFRect& diff_rect, const std::string& label) {
    ASSERT_EQ(cull_rect.CutOut(diff_rect), cull_rect) << label;
    check_empty_flips(diff_rect, label);
    check_nans(diff_rect, label);
  };

  auto reducing = [&cull_rect, &check_empty_flips, &check_nans](
                      const DlFRect& diff_rect, const DlFRect& result_rect,
                      const std::string& label) {
    ASSERT_TRUE(!result_rect.IsEmpty());
    ASSERT_EQ(cull_rect.CutOut(diff_rect), result_rect) << label;
    check_empty_flips(diff_rect, label);
    check_nans(diff_rect, label);
  };

  auto emptying = [&cull_rect, &empty_rect, &check_empty_flips, &check_nans](
                      const DlFRect& diff_rect, const std::string& label) {
    ASSERT_FALSE(cull_rect.CutOut(diff_rect).has_value()) << label;
    ASSERT_EQ(cull_rect.CutOutOrEmpty(diff_rect), empty_rect) << label;
    check_empty_flips(diff_rect, label);
    check_nans(diff_rect, label);
  };

  // Skim the corners and edge
  non_reducing(DlFRect::MakeLTRB(10, 10, 20, 20), "outside UL corner");
  non_reducing(DlFRect::MakeLTRB(20, 10, 40, 20), "Above");
  non_reducing(DlFRect::MakeLTRB(40, 10, 50, 20), "outside UR corner");
  non_reducing(DlFRect::MakeLTRB(40, 20, 50, 40), "Right");
  non_reducing(DlFRect::MakeLTRB(40, 40, 50, 50), "outside LR corner");
  non_reducing(DlFRect::MakeLTRB(20, 40, 40, 50), "Below");
  non_reducing(DlFRect::MakeLTRB(10, 40, 20, 50), "outside LR corner");
  non_reducing(DlFRect::MakeLTRB(10, 20, 20, 40), "Left");

  // Overlap corners
  non_reducing(DlFRect::MakeLTRB(15, 15, 25, 25), "covering UL corner");
  non_reducing(DlFRect::MakeLTRB(35, 15, 45, 25), "covering UR corner");
  non_reducing(DlFRect::MakeLTRB(35, 35, 45, 45), "covering LR corner");
  non_reducing(DlFRect::MakeLTRB(15, 35, 25, 45), "covering LL corner");

  // Overlap edges, but not across an entire side
  non_reducing(DlFRect::MakeLTRB(20, 15, 39, 25), "Top edge left-biased");
  non_reducing(DlFRect::MakeLTRB(21, 15, 40, 25), "Top edge, right biased");
  non_reducing(DlFRect::MakeLTRB(35, 20, 45, 39), "Right edge, top-biased");
  non_reducing(DlFRect::MakeLTRB(35, 21, 45, 40), "Right edge, bottom-biased");
  non_reducing(DlFRect::MakeLTRB(20, 35, 39, 45), "Bottom edge, left-biased");
  non_reducing(DlFRect::MakeLTRB(21, 35, 40, 45), "Bottom edge, right-biased");
  non_reducing(DlFRect::MakeLTRB(15, 20, 25, 39), "Left edge, top-biased");
  non_reducing(DlFRect::MakeLTRB(15, 21, 25, 40), "Left edge, bottom-biased");

  // Slice all the way through the middle
  non_reducing(DlFRect::MakeLTRB(25, 15, 35, 45), "Vertical interior slice");
  non_reducing(DlFRect::MakeLTRB(15, 25, 45, 35), "Horizontal interior slice");

  // Slice off each edge
  reducing(DlFRect::MakeLTRB(20, 15, 40, 25),  //
           DlFRect::MakeLTRB(20, 25, 40, 40),  //
           "Slice off top");
  reducing(DlFRect::MakeLTRB(35, 20, 45, 40),  //
           DlFRect::MakeLTRB(20, 20, 35, 40),  //
           "Slice off right");
  reducing(DlFRect::MakeLTRB(20, 35, 40, 45),  //
           DlFRect::MakeLTRB(20, 20, 40, 35),  //
           "Slice off bottom");
  reducing(DlFRect::MakeLTRB(15, 20, 25, 40),  //
           DlFRect::MakeLTRB(25, 20, 40, 40),  //
           "Slice off left");

  // cull rect contains diff rect
  non_reducing(DlFRect::MakeLTRB(21, 21, 39, 39), "Contained, non-covering");

  // cull rect equals diff rect
  emptying(cull_rect, "Perfectly covering");

  // diff rect contains cull rect
  emptying(DlFRect::MakeLTRB(15, 15, 45, 45), "Smothering");
}

}  // namespace testing
}  // namespace flutter
