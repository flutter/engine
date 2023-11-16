// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fml/logging.h"
#include "gtest/gtest.h"

#include "flutter/impeller/entity/geometry/circle_tessellator.h"
#include "flutter/impeller/geometry/geometry_asserts.h"

namespace impeller {
namespace testing {

TEST(CircleTessellator, TrigAngles) {
  {
    Trig trig(Degrees(0.0));
    EXPECT_EQ(trig.cos, 1.0);
    EXPECT_EQ(trig.sin, 0.0);
  }

  {
    Trig trig(Radians(0.0));
    EXPECT_EQ(trig.cos, 1.0);
    EXPECT_EQ(trig.sin, 0.0);
  }

  {
    Trig trig(Degrees(30.0));
    EXPECT_NEAR(trig.cos, sqrt(0.75), kEhCloseEnough);
    EXPECT_NEAR(trig.sin, 0.5, kEhCloseEnough);
  }

  {
    Trig trig(Radians(kPi / 6.0));
    EXPECT_NEAR(trig.cos, sqrt(0.75), kEhCloseEnough);
    EXPECT_NEAR(trig.sin, 0.5, kEhCloseEnough);
  }

  {
    Trig trig(Degrees(60.0));
    EXPECT_NEAR(trig.cos, 0.5, kEhCloseEnough);
    EXPECT_NEAR(trig.sin, sqrt(0.75), kEhCloseEnough);
  }

  {
    Trig trig(Radians(kPi / 3.0));
    EXPECT_NEAR(trig.cos, 0.5, kEhCloseEnough);
    EXPECT_NEAR(trig.sin, sqrt(0.75), kEhCloseEnough);
  }

  {
    Trig trig(Degrees(90.0));
    EXPECT_NEAR(trig.cos, 0.0, kEhCloseEnough);
    EXPECT_NEAR(trig.sin, 1.0, kEhCloseEnough);
  }

  {
    Trig trig(Radians(kPi / 2.0));
    EXPECT_NEAR(trig.cos, 0.0, kEhCloseEnough);
    EXPECT_NEAR(trig.sin, 1.0, kEhCloseEnough);
  }
}

TEST(CircleTessellator, DivisionVertexCounts) {
  auto test = [](Scalar pixel_radius, size_t quadrant_divisions) {
    CircleTessellator tessellator(pixel_radius);

    EXPECT_EQ(tessellator.GetQuadrantDivisionCount(), quadrant_divisions)
        << "pixel radius = " << pixel_radius;
    EXPECT_EQ(tessellator.GetCircleDivisionCount(), quadrant_divisions * 4)
        << "pixel radius = " << pixel_radius;

    EXPECT_EQ(tessellator.GetQuadrantVertexCount(), quadrant_divisions * 3)
        << "pixel radius = " << pixel_radius;
    EXPECT_EQ(tessellator.GetCircleVertexCount(), quadrant_divisions * 12)
        << "pixel radius = " << pixel_radius;
  };

  test(0.0, 1u);
  test(0.9, 1u);
  test(1.0, 2u);
  test(1.9, 2u);
  test(2.0, 6u);
  test(11.9, 6u);
  test(12.0, 9u);
  test(35.9, 9u);
  for (int i = 36; i < 140; i += 4) {
    test(i, i / 4);
    test(i + .1, i / 4 + 1);
  }
  for (int i = 140; i <= 1000; i++) {
    test(i, 35u);
  }
}

TEST(CircleTessellator, TessellationVertices) {
  auto test = [](Scalar pixel_radius, Point center, Scalar radius) {
    CircleTessellator tessellator(pixel_radius);

    auto divisions = tessellator.GetCircleDivisionCount();
    auto points = tessellator.GetCircleTriangles(center, radius);
    ASSERT_EQ(points.size(), divisions * 3);
    ASSERT_EQ(divisions % 4, 0u);

    Point expect_center = center;
    Point expect_prev = center - Point(0, radius);
    for (size_t i = 0; i < divisions; i++) {
      double angle = (k2Pi * (i + 1)) / divisions - kPiOver2;
      Point expect_next =
          center + Point(cos(angle) * radius, sin(angle) * radius);
      auto quadrant = i / (divisions / 4);
      Point test_center, test_prev, test_next;
      switch (quadrant) {
        case 0:
        case 2:
          // Quadrants 0 and 2 are ordered (center, prev, next);
          test_center = points[i * 3 + 0];
          test_prev = points[i * 3 + 1];
          test_next = points[i * 3 + 2];
          break;

        case 1:
        case 3:
          // Quadrants 1 and 3 are ordered (prev, next, center);
          test_center = points[i * 3 + 2];
          test_prev = points[i * 3 + 0];
          test_next = points[i * 3 + 1];
          break;
      }
      EXPECT_EQ(test_center, expect_center)            //
          << "point " << i << " out of " << divisions  //
          << ", center = " << center << ", radius = " << radius;
      EXPECT_POINT_NEAR(test_prev, expect_prev)        //
          << "point " << i << " out of " << divisions  //
          << ", center = " << center << ", radius = " << radius;
      EXPECT_POINT_NEAR(test_next, expect_next)        //
          << "point " << i << " out of " << divisions  //
          << ", center = " << center << ", radius = " << radius;
      expect_prev = expect_next;
    }
    EXPECT_POINT_NEAR(expect_prev, center - Point(0, radius));
  };

  test(2.0, {}, 2.0);
  test(2.0, {10, 10}, 2.0);
  test(1000.0, {}, 2.0);
  test(2.0, {}, 1000.0);
}

}  // namespace testing
}  // namespace impeller
