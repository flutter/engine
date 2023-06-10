// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_region.h"
#include "gtest/gtest.h"

#include "third_party/skia/include/core/SkRegion.h"

#include <random>

namespace flutter {
namespace testing {

TEST(DisplayListRegion, EmptyRegion) {
  DlRegion region(std::vector<SkIRect>{});
  EXPECT_TRUE(region.getRects().empty());
}

TEST(DisplayListRegion, SingleRectangle) {
  DlRegion region({SkIRect::MakeLTRB(10, 10, 50, 50)});
  auto rects = region.getRects();
  ASSERT_EQ(rects.size(), 1u);
  EXPECT_EQ(rects.front(), SkIRect::MakeLTRB(10, 10, 50, 50));
}

TEST(DisplayListRegion, NonOverlappingRectangles1) {
  std::vector<SkIRect> rects_in;
  for (int i = 0; i < 10; ++i) {
    SkIRect rect = SkIRect::MakeXYWH(50 * i, 50 * i, 50, 50);
    rects_in.push_back(rect);
  }
  DlRegion region(rects_in);
  auto rects = region.getRects();
  std::vector<SkIRect> expected{
      {0, 0, 50, 50},       {50, 50, 100, 100},   {100, 100, 150, 150},
      {150, 150, 200, 200}, {200, 200, 250, 250}, {250, 250, 300, 300},
      {300, 300, 350, 350}, {350, 350, 400, 400}, {400, 400, 450, 450},
      {450, 450, 500, 500},
  };
  EXPECT_EQ(rects, expected);
}

TEST(DisplayListRegion, NonOverlappingRectangles2) {
  DlRegion region({
      SkIRect::MakeXYWH(5, 5, 10, 10),
      SkIRect::MakeXYWH(25, 5, 10, 10),
      SkIRect::MakeXYWH(5, 25, 10, 10),
      SkIRect::MakeXYWH(25, 25, 10, 10),
  });
  auto rects = region.getRects();
  std::vector<SkIRect> expected{
      SkIRect::MakeXYWH(5, 5, 10, 10),
      SkIRect::MakeXYWH(25, 5, 10, 10),
      SkIRect::MakeXYWH(5, 25, 10, 10),
      SkIRect::MakeXYWH(25, 25, 10, 10),
  };
  EXPECT_EQ(rects, expected);
}

TEST(DisplayListRegion, NonOverlappingRectangles3) {
  DlRegion region({
      SkIRect::MakeXYWH(0, 0, 10, 10),
      SkIRect::MakeXYWH(-11, -11, 10, 10),
      SkIRect::MakeXYWH(11, 11, 10, 10),
      SkIRect::MakeXYWH(-11, 0, 10, 10),
      SkIRect::MakeXYWH(0, 11, 10, 10),
      SkIRect::MakeXYWH(0, -11, 10, 10),
      SkIRect::MakeXYWH(11, 0, 10, 10),
      SkIRect::MakeXYWH(11, -11, 10, 10),
      SkIRect::MakeXYWH(-11, 11, 10, 10),
  });
  auto rects = region.getRects();
  std::vector<SkIRect> expected{
      SkIRect::MakeXYWH(-11, -11, 10, 10),  //
      SkIRect::MakeXYWH(0, -11, 10, 10),    //
      SkIRect::MakeXYWH(11, -11, 10, 10),   //
      SkIRect::MakeXYWH(-11, 0, 10, 10),    //
      SkIRect::MakeXYWH(0, 0, 10, 10),      //
      SkIRect::MakeXYWH(11, 0, 10, 10),     //
      SkIRect::MakeXYWH(-11, 11, 10, 10),   //
      SkIRect::MakeXYWH(0, 11, 10, 10),     //
      SkIRect::MakeXYWH(11, 11, 10, 10),
  };
  EXPECT_EQ(rects, expected);
}

TEST(DisplayListRegion, MergeTouchingRectangles) {
  DlRegion region({
      SkIRect::MakeXYWH(0, 0, 10, 10),
      SkIRect::MakeXYWH(-10, -10, 10, 10),
      SkIRect::MakeXYWH(10, 10, 10, 10),
      SkIRect::MakeXYWH(-10, 0, 10, 10),
      SkIRect::MakeXYWH(0, 10, 10, 10),
      SkIRect::MakeXYWH(0, -10, 10, 10),
      SkIRect::MakeXYWH(10, 0, 10, 10),
      SkIRect::MakeXYWH(10, -10, 10, 10),
      SkIRect::MakeXYWH(-10, 10, 10, 10),
  });

  auto rects = region.getRects();
  std::vector<SkIRect> expected{
      SkIRect::MakeXYWH(-10, -10, 30, 30),
  };
  EXPECT_EQ(rects, expected);
}

TEST(DisplayListRegion, OverlappingRectangles) {
  std::vector<SkIRect> rects_in;
  for (int i = 0; i < 10; ++i) {
    SkIRect rect = SkIRect::MakeXYWH(10 * i, 10 * i, 50, 50);
    rects_in.push_back(rect);
  }
  DlRegion region(rects_in);
  auto rects = region.getRects();
  std::vector<SkIRect> expected{
      {0, 0, 50, 10},      {0, 10, 60, 20},     {0, 20, 70, 30},
      {0, 30, 80, 40},     {0, 40, 90, 50},     {10, 50, 100, 60},
      {20, 60, 110, 70},   {30, 70, 120, 80},   {40, 80, 130, 90},
      {50, 90, 140, 100},  {60, 100, 140, 110}, {70, 110, 140, 120},
      {80, 120, 140, 130}, {90, 130, 140, 140},
  };

  EXPECT_EQ(rects, expected);
}

TEST(DisplayListRegion, Deband) {
  DlRegion region({
      SkIRect::MakeXYWH(0, 0, 50, 50),
      SkIRect::MakeXYWH(60, 0, 20, 20),
      SkIRect::MakeXYWH(90, 0, 50, 50),
  });

  auto rects_with_deband = region.getRects(true);
  std::vector<SkIRect> expected{
      SkIRect::MakeXYWH(60, 0, 20, 20),
      SkIRect::MakeXYWH(0, 0, 50, 50),
      SkIRect::MakeXYWH(90, 0, 50, 50),
  };
  EXPECT_EQ(rects_with_deband, expected);

  auto rects_without_deband = region.getRects(false);
  std::vector<SkIRect> expected_without_deband{
      SkIRect::MakeXYWH(0, 0, 50, 20),   //
      SkIRect::MakeXYWH(60, 0, 20, 20),  //
      SkIRect::MakeXYWH(90, 0, 50, 20),  //
      SkIRect::MakeXYWH(0, 20, 50, 30),  //
      SkIRect::MakeXYWH(90, 20, 50, 30),
  };
  EXPECT_EQ(rects_without_deband, expected_without_deband);
}

TEST(DisplayListRegion, Intersects1) {
  DlRegion region1({
      SkIRect::MakeXYWH(0, 0, 20, 20),
      SkIRect::MakeXYWH(20, 20, 20, 20),
  });
  DlRegion region2({
      SkIRect::MakeXYWH(20, 0, 20, 20),
      SkIRect::MakeXYWH(0, 20, 20, 20),
  });
  EXPECT_FALSE(region1.intersects(region2));
  EXPECT_FALSE(region2.intersects(region1));

  EXPECT_TRUE(region1.intersects(region2.bounds()));
  EXPECT_TRUE(region2.intersects(region1.bounds()));

  EXPECT_TRUE(region1.intersects(SkIRect::MakeXYWH(0, 0, 20, 20)));
  EXPECT_FALSE(region1.intersects(SkIRect::MakeXYWH(20, 0, 20, 20)));

  EXPECT_TRUE(region1.intersects(
      DlRegion(std::vector<SkIRect>{SkIRect::MakeXYWH(0, 0, 20, 20)})));
  EXPECT_FALSE(region1.intersects(
      DlRegion(std::vector<SkIRect>{SkIRect::MakeXYWH(20, 0, 20, 20)})));

  EXPECT_FALSE(region1.intersects(SkIRect::MakeXYWH(-1, -1, 1, 1)));
  EXPECT_TRUE(region1.intersects(SkIRect::MakeXYWH(0, 0, 1, 1)));

  EXPECT_FALSE(region1.intersects(SkIRect::MakeXYWH(40, 40, 1, 1)));
  EXPECT_TRUE(region1.intersects(SkIRect::MakeXYWH(39, 39, 1, 1)));
}

TEST(DisplayListRegion, Intersects2) {
  DlRegion region1({
      SkIRect::MakeXYWH(-10, -10, 20, 20),
      SkIRect::MakeXYWH(-30, -30, 20, 20),
  });
  DlRegion region2({
      SkIRect::MakeXYWH(20, 20, 5, 5),
      SkIRect::MakeXYWH(0, 0, 20, 20),
  });
  EXPECT_TRUE(region1.intersects(region2));
  EXPECT_TRUE(region2.intersects(region1));
}

TEST(DisplayListRegion, Union1) {
  DlRegion region1({
      SkIRect::MakeXYWH(0, 0, 20, 20),
      SkIRect::MakeXYWH(20, 20, 20, 20),
  });
  DlRegion region2({
      SkIRect::MakeXYWH(20, 0, 20, 20),
      SkIRect::MakeXYWH(0, 20, 20, 20),
  });
  DlRegion u = DlRegion::MakeUnion(region1, region2);
  EXPECT_EQ(u.bounds(), SkIRect::MakeXYWH(0, 0, 40, 40));
  auto rects = u.getRects();
  std::vector<SkIRect> expected{
      SkIRect::MakeXYWH(0, 0, 40, 40),  //
  };
  EXPECT_EQ(rects, expected);
}

TEST(DisplayListRegion, Union2) {
  DlRegion region1({
      SkIRect::MakeXYWH(0, 0, 20, 20),
      SkIRect::MakeXYWH(21, 21, 20, 20),
  });
  DlRegion region2({
      SkIRect::MakeXYWH(21, 0, 20, 20),
      SkIRect::MakeXYWH(0, 21, 20, 20),
  });
  DlRegion u = DlRegion::MakeUnion(region1, region2);
  EXPECT_EQ(u.bounds(), SkIRect::MakeXYWH(0, 0, 41, 41));
  auto rects = u.getRects();
  std::vector<SkIRect> expected{
      SkIRect::MakeXYWH(0, 0, 20, 20),
      SkIRect::MakeXYWH(21, 0, 20, 20),
      SkIRect::MakeXYWH(0, 21, 20, 20),
      SkIRect::MakeXYWH(21, 21, 20, 20),
  };
  EXPECT_EQ(rects, expected);
}

TEST(DisplayListRegion, Union3) {
  DlRegion region1({
      SkIRect::MakeXYWH(-10, -10, 20, 20),
  });
  DlRegion region2({
      SkIRect::MakeXYWH(0, 0, 20, 20),
  });
  DlRegion u = DlRegion::MakeUnion(region1, region2);
  EXPECT_EQ(u.bounds(), SkIRect::MakeXYWH(-10, -10, 30, 30));
  auto rects = u.getRects();
  std::vector<SkIRect> expected{
      SkIRect::MakeXYWH(-10, -10, 20, 10),
      SkIRect::MakeXYWH(-10, 0, 30, 10),
      SkIRect::MakeXYWH(0, 10, 20, 10),
  };
  EXPECT_EQ(rects, expected);
  // for (auto& rect : rects) {
  //   printf("SkIRect::MakeXYWH(%d, %d, %d, %d,),\n", rect.fLeft, rect.fTop,
  //   rect.width(),
  //          rect.height());
  // }
}

TEST(DisplayListRegion, UnionEmpty) {
  {
    DlRegion region1(std::vector<SkIRect>{});
    DlRegion region2(std::vector<SkIRect>{});
    DlRegion u = DlRegion::MakeUnion(region1, region2);
    EXPECT_EQ(u.bounds(), SkIRect::MakeEmpty());
    auto rects = u.getRects();
    std::vector<SkIRect> expected{};
    EXPECT_EQ(rects, expected);
  }
  {
    DlRegion region1(std::vector<SkIRect>{});
    DlRegion region2({
        SkIRect::MakeXYWH(0, 0, 20, 20),
    });
    DlRegion u = DlRegion::MakeUnion(region1, region2);
    EXPECT_EQ(u.bounds(), SkIRect::MakeXYWH(0, 0, 20, 20));
    auto rects = u.getRects();
    std::vector<SkIRect> expected{
        SkIRect::MakeXYWH(0, 0, 20, 20),
    };
  }
  {
    DlRegion region1({
        SkIRect::MakeXYWH(0, 0, 20, 20),
    });
    DlRegion region2(std::vector<SkIRect>{});
    DlRegion u = DlRegion::MakeUnion(region1, region2);
    EXPECT_EQ(u.bounds(), SkIRect::MakeXYWH(0, 0, 20, 20));
    auto rects = u.getRects();
    std::vector<SkIRect> expected{
        SkIRect::MakeXYWH(0, 0, 20, 20),
    };
  }
}

void CheckEquality(const DlRegion& dl_region, const SkRegion& sk_region) {
  EXPECT_EQ(dl_region.bounds(), sk_region.getBounds());

  // Do not deband the rectangles - identical to SkRegion::Iterator
  auto rects = dl_region.getRects(false);

  std::vector<SkIRect> skia_rects;

  auto iterator = SkRegion::Iterator(sk_region);
  while (!iterator.done()) {
    skia_rects.push_back(iterator.rect());
    iterator.next();
  }

  EXPECT_EQ(rects, skia_rects);
}

TEST(DisplayListRegion, TestAgainstSkRegion) {
  struct Settings {
    int max_size;
    size_t iteration_count;
  };
  std::vector<Settings> all_settings{
      {100, 1},     //
      {100, 10},    //
      {100, 100},   //
      {100, 1000},  //
      {400, 10},    //
      {400, 100},   //
      {400, 1000},  //
      {800, 10},    //
      {800, 100},   //
      {800, 1000},
  };

  for (const auto& settings : all_settings) {
    std::random_device d;
    std::seed_seq seed{::testing::UnitTest::GetInstance()->random_seed()};
    std::mt19937 rng(seed);

    SkRegion sk_region1;
    SkRegion sk_region2;

    std::uniform_int_distribution pos(0, 4000);
    std::uniform_int_distribution size(1, settings.max_size);

    std::vector<SkIRect> rects_in1;
    std::vector<SkIRect> rects_in2;

    for (size_t i = 0; i < settings.iteration_count; ++i) {
      SkIRect rect =
          SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
      rects_in1.push_back(rect);
      sk_region1.op(rect, SkRegion::kUnion_Op);

      rect = SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
      rects_in2.push_back(rect);
      sk_region2.op(rect, SkRegion::kUnion_Op);
    }

    DlRegion region1(rects_in1);
    CheckEquality(region1, sk_region1);

    DlRegion region2(rects_in2);
    CheckEquality(region2, sk_region2);

    auto intersects_1 = region1.intersects(region2);
    auto intersects_2 = region2.intersects(region1);
    auto sk_intesects = sk_region1.intersects(sk_region2);
    EXPECT_EQ(intersects_1, intersects_2);
    EXPECT_EQ(intersects_1, sk_intesects);

    {
      auto rects = region2.getRects(true);
      for (const auto& r : rects) {
        EXPECT_EQ(region1.intersects(r), sk_region1.intersects(r));
      }
    }

    DlRegion dl_union = DlRegion::MakeUnion(region1, region2);
    SkRegion sk_union(sk_region1);
    sk_union.op(sk_region2, SkRegion::kUnion_Op);
    CheckEquality(dl_union, sk_union);

    DlRegion dl_intersection = DlRegion::MakeIntersection(region1, region2);
    SkRegion sk_intersection(sk_region1);
    sk_intersection.op(sk_region2, SkRegion::kIntersect_Op);
    CheckEquality(dl_intersection, sk_intersection);
  }
}

}  // namespace testing
}  // namespace flutter
