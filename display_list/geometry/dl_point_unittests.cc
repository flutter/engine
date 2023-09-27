// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_point.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlPointTest, FPointEmptyConstructor) {
  DlFPoint pt;
  ASSERT_EQ(pt.x(), 0.0f);
  ASSERT_EQ(pt.y(), 0.0f);
  ASSERT_EQ(pt.GetLength(), 0.0f);
  ASSERT_EQ(pt.GetLengthSquared(), 0.0f);
  ASSERT_EQ(pt.GetDistance({}), 0.0f);
  ASSERT_EQ(pt.GetDistanceSquared({}), 0.0f);
  ASSERT_EQ(pt.Normalize(), DlFPoint(1.0f, 0.0f));
  ASSERT_TRUE(pt.IsFinite());
}

TEST(DlPointTest, IPointEmptyConstructor) {
  DlIPoint pt;
  ASSERT_EQ(pt.x(), 0);
  ASSERT_EQ(pt.y(), 0);
  ASSERT_EQ(pt.GetLength(), 0.0f);
  ASSERT_EQ(pt.GetLengthSquared(), 0.0f);
  ASSERT_EQ(pt.GetDistance({}), 0.0f);
  ASSERT_EQ(pt.GetDistanceSquared({}), 0.0f);
  // ASSERT_EQ(pt.Normalize(), pt); // should fail to compile
  // ASSERT_TRUE(pt.IsFinite()); // should fail to compile
}

TEST(DlPointTest, FPointDefaultConstructor) {
  DlFPoint pt = DlFPoint();
  ASSERT_EQ(pt.x(), 0.0f);
  ASSERT_EQ(pt.y(), 0.0f);
  ASSERT_EQ(pt.GetLength(), 0.0f);
  ASSERT_EQ(pt.GetLengthSquared(), 0.0f);
  ASSERT_EQ(pt.GetDistance({}), 0.0f);
  ASSERT_EQ(pt.GetDistanceSquared({}), 0.0f);
  ASSERT_EQ(pt.Normalize(), DlFPoint(1.0f, 0.0f));
  ASSERT_TRUE(pt.IsFinite());
}

TEST(DlPointTest, IPointDefaultConstructor) {
  DlIPoint pt = DlIPoint();
  ASSERT_EQ(pt.x(), 0);
  ASSERT_EQ(pt.y(), 0);
  ASSERT_EQ(pt.GetLength(), 0.0f);
  ASSERT_EQ(pt.GetLengthSquared(), 0.0f);
  ASSERT_EQ(pt.GetDistance({}), 0.0f);
  ASSERT_EQ(pt.GetDistanceSquared({}), 0.0f);
  // ASSERT_EQ(pt.Normalize(), pt); // should fail to compile
  // ASSERT_TRUE(pt.IsFinite()); // should fail to compile
}

TEST(DlPointTest, FPointXYConstructor) {
  DlFPoint pt = DlFPoint(3.0f, 4.0f);
  ASSERT_EQ(pt.x(), 3.0f);
  ASSERT_EQ(pt.y(), 4.0f);
  ASSERT_EQ(pt.GetLength(), 5.0f);
  ASSERT_EQ(pt.GetLengthSquared(), 25.0f);
  ASSERT_EQ(pt.GetDistance({}), 5.0f);
  ASSERT_EQ(pt.GetDistanceSquared({}), 25.0f);
  ASSERT_EQ(pt.Normalize(), DlFPoint(0.6f, 0.8f));
  ASSERT_TRUE(pt.IsFinite());
}

TEST(DlPointTest, IPointXYConstructor) {
  DlIPoint pt = DlIPoint(3, 4);
  ASSERT_EQ(pt.x(), 3);
  ASSERT_EQ(pt.y(), 4);
  ASSERT_EQ(pt.GetLength(), 5.0f);
  ASSERT_EQ(pt.GetLengthSquared(), 25.0f);
  ASSERT_EQ(pt.GetDistance({}), 5.0f);
  ASSERT_EQ(pt.GetDistanceSquared({}), 25.0f);
  // ASSERT_EQ(pt.Normalize(), pt); // should fail to compile
  // ASSERT_TRUE(pt.IsFinite()); // should fail to compile
}

TEST(DlPointTest, FPointFPointConstructor) {
  DlFPoint src = DlFPoint(3.0f, 4.0f);
  DlFPoint pt = DlFPoint(src);
  ASSERT_EQ(pt.x(), 3.0f);
  ASSERT_EQ(pt.y(), 4.0f);
  ASSERT_EQ(pt.GetLength(), 5.0f);
  ASSERT_EQ(pt.GetLengthSquared(), 25.0f);
  ASSERT_EQ(pt.GetDistance({}), 5.0f);
  ASSERT_EQ(pt.GetDistanceSquared({}), 25.0f);
  ASSERT_EQ(pt.Normalize(), DlFPoint(0.6f, 0.8f));
  ASSERT_TRUE(pt.IsFinite());
}

TEST(DlPointTest, IPointIPointConstructor) {
  DlIPoint src = DlIPoint(3, 4);
  DlIPoint pt = DlIPoint(src);
  ASSERT_EQ(pt.x(), 3);
  ASSERT_EQ(pt.y(), 4);
  ASSERT_EQ(pt.GetLength(), 5.0f);
  ASSERT_EQ(pt.GetLengthSquared(), 25.0f);
  ASSERT_EQ(pt.GetDistance({}), 5.0f);
  ASSERT_EQ(pt.GetDistanceSquared({}), 25.0f);
  // ASSERT_EQ(pt.Normalize(), pt); // should fail to compile
  // ASSERT_TRUE(pt.IsFinite()); // should fail to compile
}

// The data for the Length/Distance tests comes from the first 3 unique
// pythagorean triples (3, 4, 5), (5, 12, 13), (8, 15, 17)
TEST(DlPointTest, FPointGetLength) {
  auto test = [](DlScalar a, DlScalar b, DlScalar c) {
    std::string label = "|" + std::to_string(a) + ", " + std::to_string(b) +
                        "| == " + std::to_string(c);
    ASSERT_EQ(DlFPoint(a, b).GetLength(), c) << label;
    ASSERT_EQ(DlFPoint(a, b).GetLengthSquared(), c * c) << label;
  };

  test(3, 4, 5);
  test(5, 12, 13);
  test(8, 15, 17);
}

TEST(DlPointTest, IPointGetLength) {
  auto test = [](DlInt a, DlInt b, DlScalar c) {
    std::string label = "|" + std::to_string(a) + ", " + std::to_string(b) +
                        "| == " + std::to_string(c);
    ASSERT_EQ(DlIPoint(a, b).GetLength(), c) << label;
    ASSERT_EQ(DlIPoint(a, b).GetLengthSquared(), c * c) << label;
  };

  test(3, 4, 5);
  test(5, 12, 13);
  test(8, 15, 17);
}

TEST(DlPointTest, FPointGetDistance) {
  auto test = [](DlScalar a, DlScalar b, DlScalar c, DlScalar x, DlScalar y) {
    DlFPoint pt1 = DlFPoint(x, y);
    DlFPoint pt2 = DlFPoint(x + a, y + b);
    std::stringstream stream;
    stream << "| " << pt1 << " => " << pt2 << "| = " << c;
    auto label = stream.str();
    ASSERT_EQ(pt1.GetDistance(pt2), c) << label;
    ASSERT_EQ(pt1.GetDistanceSquared(pt2), c * c) << label;
    ASSERT_EQ(pt2.GetDistance(pt1), c) << label;
    ASSERT_EQ(pt2.GetDistanceSquared(pt1), c * c) << label;
  };

  for (int x = -10; x <= 10; x++) {
    for (int y = -10; y <= 10; y++) {
      test(3, 4, 5, x, y);
      test(5, 12, 13, x, y);
      test(8, 15, 17, x, y);
    }
  }
}

TEST(DlPointTest, IPointGetDistance) {
  auto test = [](DlInt a, DlInt b, DlScalar c, DlInt x, DlInt y) {
    DlIPoint pt1 = DlIPoint(x, y);
    DlIPoint pt2 = DlIPoint(x + a, y + b);
    std::stringstream stream;
    stream << "| " << pt1 << " => " << pt2 << "| = " << c;
    auto label = stream.str();
    ASSERT_EQ(pt1.GetDistance(pt2), c) << label;
    ASSERT_EQ(pt1.GetDistanceSquared(pt2), c * c) << label;
    ASSERT_EQ(pt2.GetDistance(pt1), c) << label;
    ASSERT_EQ(pt2.GetDistanceSquared(pt1), c * c) << label;
  };

  for (int x = -10; x <= 10; x++) {
    for (int y = -10; y <= 10; y++) {
      test(3, 4, 5, x, y);
      test(5, 12, 13, x, y);
      test(8, 15, 17, x, y);
    }
  }
}

TEST(DlPointTest, NaNInfinityCoordinates) {
  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  DlScalar pos_inf = std::numeric_limits<DlScalar>::infinity();
  DlScalar neg_inf = -std::numeric_limits<DlScalar>::infinity();

  DlFPoint pt = DlFPoint(2.0f, 4.0f);
  ASSERT_TRUE(pt.IsFinite());

  auto test_non_finite = [](const DlFPoint& pt, int mask, DlScalar non_finite) {
    ASSERT_TRUE(pt.IsFinite());
    std::string label = "with " + std::to_string(non_finite) +  //
                        " in " + std::to_string(mask);

    DlScalar x = (mask & (1 << 0)) ? non_finite : pt.x();
    DlScalar y = (mask & (1 << 1)) ? non_finite : pt.y();

    DlFPoint pt_nonf = DlFPoint(x, y);
    if (mask == 0) {
      ASSERT_TRUE(pt_nonf.IsFinite()) << label;
      ASSERT_EQ(pt, pt_nonf) << label;
    } else {
      ASSERT_FALSE(pt_nonf.IsFinite()) << label;
      ASSERT_NE(pt, pt_nonf) << label;
    }
  };

  for (int mask = 0; mask <= 0x3; mask++) {
    test_non_finite(pt, mask, nan);
    test_non_finite(pt, mask, pos_inf);
    test_non_finite(pt, mask, neg_inf);
  }
}

}  // namespace testing
}  // namespace flutter
