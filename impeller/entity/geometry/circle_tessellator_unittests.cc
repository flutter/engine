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
  auto test = [](const Matrix& matrix, Scalar radius,
                 size_t quadrant_divisions) {
    CircleTessellator tessellator(matrix, radius);

    EXPECT_EQ(tessellator.GetQuadrantDivisionCount(), quadrant_divisions)
        << "transform = " << matrix << ", radius = " << radius;

    EXPECT_EQ(tessellator.GetCircleVertexCount(), (quadrant_divisions + 1) * 4)
        << "transform = " << matrix << ", radius = " << radius;
  };

  test({}, 0.0, 1u);
  test({}, 0.9, 1u);
  test({}, 1.0, 2u);
  test({}, 1.9, 2u);
  test(Matrix::MakeScale(Vector2(2.0, 2.0)), 0.95, 2u);
  test({}, 2.0, 6u);
  test(Matrix::MakeScale(Vector2(2.0, 2.0)), 1.0, 6u);
  test({}, 11.9, 6u);
  test({}, 12.0, 9u);
  test({}, 35.9, 9u);
  for (int i = 36; i < 140; i += 4) {
    test({}, i, i / 4);
    test({}, i + .1, i / 4 + 1);
  }
  for (int i = 140; i <= 1000; i++) {
    test({}, i, 35u);
  }
}

TEST(CircleTessellator, CircleTessellationVertices) {
  auto test = [](Scalar pixel_radius, Point center, Scalar radius) {
    CircleTessellator tessellator({}, pixel_radius);

    auto vertex_count = tessellator.GetCircleVertexCount();
    auto vertices = std::vector<Point>();
    tessellator.GenerateCircleTriangleStrip(
        [&vertices](const Point& p) {  //
          vertices.push_back(p);
        },
        center, radius);
    ASSERT_EQ(vertices.size(), vertex_count);
    ASSERT_EQ(vertex_count % 4, 0u);

    auto quadrant_count = vertex_count / 4;
    for (size_t i = 0; i < quadrant_count; i++) {
      double angle = kPiOver2 * i / (quadrant_count - 1);
      double rsin = sin(angle) * radius;
      double rcos = cos(angle) * radius;
      EXPECT_POINT_NEAR(vertices[i * 2],
                        Point(center.x - rcos, center.y + rsin))
          << "vertex " << i << ", angle = " << angle * 180.0 / kPi << std::endl;
      EXPECT_POINT_NEAR(vertices[i * 2 + 1],
                        Point(center.x - rcos, center.y - rsin))
          << "vertex " << i << ", angle = " << angle * 180.0 / kPi << std::endl;
      EXPECT_POINT_NEAR(vertices[vertex_count - i * 2 - 1],
                        Point(center.x + rcos, center.y - rsin))
          << "vertex " << i << ", angle = " << angle * 180.0 / kPi << std::endl;
      EXPECT_POINT_NEAR(vertices[vertex_count - i * 2 - 2],
                        Point(center.x + rcos, center.y + rsin))
          << "vertex " << i << ", angle = " << angle * 180.0 / kPi << std::endl;
    }
  };

  test(2.0, {}, 2.0);
  test(2.0, {10, 10}, 2.0);
  test(1000.0, {}, 2.0);
  test(2.0, {}, 1000.0);
}

}  // namespace testing
}  // namespace impeller
