// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/entity/geometry/geometry.h"
#include "impeller/entity/geometry/stroke_path_geometry.h"
#include "impeller/geometry/geometry_asserts.h"
#include "impeller/geometry/path_builder.h"

inline void CheckSolidVerticesNear(
    std::vector<impeller::SolidFillVertexShader::PerVertexData> a,
    std::vector<impeller::SolidFillVertexShader::PerVertexData> b,
    const std::string& file,
    int line) {
  std::string label = "from " + file + ":" + std::to_string(line);
  EXPECT_EQ(a.size(), b.size()) << label;
  for (auto i = 0u; i < std::min(a.size(), b.size()); i++) {
    EXPECT_POINT_NEAR(a[i].position, b[i].position)
        << "vertex " << i << " " << label;
  }
}

inline void CheckTextureVerticesNear(
    std::vector<impeller::TextureFillVertexShader::PerVertexData> a,
    std::vector<impeller::TextureFillVertexShader::PerVertexData> b,
    const std::string& file = "",
    int line = 0) {
  std::string label = "from " + file + ":" + std::to_string(line);
  EXPECT_EQ(a.size(), b.size()) << label;
  for (auto i = 0u; i < std::min(a.size(), b.size()); i++) {
    EXPECT_POINT_NEAR(a[i].position, b[i].position)
        << "vertex " << i << " " << label;
    EXPECT_POINT_NEAR(a[i].texture_coords, b[i].texture_coords)
        << "vertex " << i << " " << label;
  }
}

#define EXPECT_SOLID_VERTICES_NEAR(a, b) \
  CheckSolidVerticesNear(a, b, __FILE__, __LINE__)
#define EXPECT_TEXTURE_VERTICES_NEAR(a, b) \
  CheckTextureVerticesNear(a, b, __FILE__, __LINE__)

namespace impeller {

class ImpellerEntityUnitTestAccessor {
 public:
  static std::vector<SolidFillVertexShader::PerVertexData>
  GenerateSolidStrokeVertices(const Path::Polyline& polyline,
                              Scalar stroke_width,
                              Scalar miter_limit,
                              Join stroke_join,
                              Cap stroke_cap,
                              Scalar scale) {
    return StrokePathGeometry::GenerateSolidStrokeVertices(
        polyline, stroke_width, miter_limit, stroke_join, stroke_cap, scale);
  }

  static std::vector<TextureFillVertexShader::PerVertexData>
  GenerateSolidStrokeVerticesUV(const Path::Polyline& polyline,
                                Scalar stroke_width,
                                Scalar miter_limit,
                                Join stroke_join,
                                Cap stroke_cap,
                                Scalar scale,
                                Point texture_origin,
                                Size texture_size,
                                const Matrix& effect_transform) {
    return StrokePathGeometry::GenerateSolidStrokeVerticesUV(
        polyline, stroke_width, miter_limit, stroke_join, stroke_cap, scale,
        texture_origin, texture_size, effect_transform);
  }
};

namespace testing {

TEST(EntityGeometryTest, RectGeometryCoversArea) {
  auto geometry = Geometry::MakeRect(Rect::MakeLTRB(0, 0, 100, 100));
  ASSERT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(0, 0, 100, 100)));
  ASSERT_FALSE(geometry->CoversArea({}, Rect::MakeLTRB(-1, 0, 100, 100)));
  ASSERT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(1, 1, 100, 100)));
  ASSERT_TRUE(geometry->CoversArea({}, Rect()));
}

TEST(EntityGeometryTest, FillPathGeometryCoversArea) {
  auto path = PathBuilder{}.AddRect(Rect::MakeLTRB(0, 0, 100, 100)).TakePath();
  auto geometry = Geometry::MakeFillPath(
      path, /* inner rect */ Rect::MakeLTRB(0, 0, 100, 100));
  ASSERT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(0, 0, 100, 100)));
  ASSERT_FALSE(geometry->CoversArea({}, Rect::MakeLTRB(-1, 0, 100, 100)));
  ASSERT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(1, 1, 100, 100)));
  ASSERT_TRUE(geometry->CoversArea({}, Rect()));
}

TEST(EntityGeometryTest, FillPathGeometryCoversAreaNoInnerRect) {
  auto path = PathBuilder{}.AddRect(Rect::MakeLTRB(0, 0, 100, 100)).TakePath();
  auto geometry = Geometry::MakeFillPath(path);
  ASSERT_FALSE(geometry->CoversArea({}, Rect::MakeLTRB(0, 0, 100, 100)));
  ASSERT_FALSE(geometry->CoversArea({}, Rect::MakeLTRB(-1, 0, 100, 100)));
  ASSERT_FALSE(geometry->CoversArea({}, Rect::MakeLTRB(1, 1, 100, 100)));
  ASSERT_FALSE(geometry->CoversArea({}, Rect()));
}

TEST(EntityGeometryTest, LineGeometryCoverage) {
  {
    auto geometry = Geometry::MakeLine({10, 10}, {20, 10}, 2, Cap::kButt);
    EXPECT_EQ(geometry->GetCoverage({}), Rect::MakeLTRB(10, 9, 20, 11));
    EXPECT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(10, 9, 20, 11)));
  }

  {
    auto geometry = Geometry::MakeLine({10, 10}, {20, 10}, 2, Cap::kSquare);
    EXPECT_EQ(geometry->GetCoverage({}), Rect::MakeLTRB(9, 9, 21, 11));
    EXPECT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(9, 9, 21, 11)));
  }

  {
    auto geometry = Geometry::MakeLine({10, 10}, {10, 20}, 2, Cap::kButt);
    EXPECT_EQ(geometry->GetCoverage({}), Rect::MakeLTRB(9, 10, 11, 20));
    EXPECT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(9, 10, 11, 20)));
  }

  {
    auto geometry = Geometry::MakeLine({10, 10}, {10, 20}, 2, Cap::kSquare);
    EXPECT_EQ(geometry->GetCoverage({}), Rect::MakeLTRB(9, 9, 11, 21));
    EXPECT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(9, 9, 11, 21)));
  }
}

TEST(EntityGeometryTest, RoundRectGeometryCoversArea) {
  auto geometry =
      Geometry::MakeRoundRect(Rect::MakeLTRB(0, 0, 100, 100), Size(20, 20));
  EXPECT_FALSE(geometry->CoversArea({}, Rect::MakeLTRB(15, 15, 85, 85)));
  EXPECT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(20, 20, 80, 80)));
  EXPECT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(30, 1, 70, 99)));
  EXPECT_TRUE(geometry->CoversArea({}, Rect::MakeLTRB(1, 30, 99, 70)));
}

TEST(EntityGeometryTest, StrokePathGeometryTransformOfLine) {
  auto path =
      PathBuilder().AddLine(Point(100, 100), Point(200, 100)).TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kBevel, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);

  {
    auto uv_vertices =
        ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVerticesUV(
            polyline, 10.0f, 10.0f, Join::kBevel, Cap::kButt, 1.0,  //
            Point(50.0f, 40.0f), Size(20.0f, 40.0f),
            Matrix::MakeScale({8.0f, 4.0f, 1.0f}));
    // uvx = ((x * 8) - 50) / 20
    // uvy = ((y * 4) - 40) / 40
    auto uv = [](const Point& p) {
      return Point(((p.x * 8.0f) - 50.0f) / 20.0f,
                   ((p.y * 4.0f) - 40.0f) / 40.0f);
    };
    std::vector<TextureFillVertexShader::PerVertexData> uv_expected;
    for (size_t i = 0; i < expected.size(); i++) {
      auto p = expected[i].position;
      uv_expected.push_back({.position = p, .texture_coords = uv(p)});
    }

    EXPECT_TEXTURE_VERTICES_NEAR(uv_vertices, uv_expected);
  }

  {
    auto uv_vertices =
        ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVerticesUV(
            polyline, 10.0f, 10.0f, Join::kBevel, Cap::kButt, 1.0,  //
            Point(50.0f, 40.0f), Size(20.0f, 40.0f),
            Matrix::MakeTranslation({8.0f, 4.0f}));
    // uvx = ((x + 8) - 50) / 20
    // uvy = ((y + 4) - 40) / 40
    auto uv = [](const Point& p) {
      return Point(((p.x + 8.0f) - 50.0f) / 20.0f,
                   ((p.y + 4.0f) - 40.0f) / 40.0f);
    };
    std::vector<TextureFillVertexShader::PerVertexData> uv_expected;
    for (size_t i = 0; i < expected.size(); i++) {
      auto p = expected[i].position;
      uv_expected.push_back({.position = p, .texture_coords = uv(p)});
    }

    EXPECT_TEXTURE_VERTICES_NEAR(uv_vertices, uv_expected);
  }
}

TEST(EntityGeometryTest, StrokePathGeometryButtCaps) {
  auto path =
      PathBuilder().AddLine(Point(100, 100), Point(200, 100)).TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kBevel, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometrySquareCaps) {
  auto path =
      PathBuilder().AddLine(Point(100, 100), Point(200, 100)).TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kBevel, Cap::kSquare, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(95.0f, 105.0f)},   //
      {.position = Point(95.0f, 95.0f)},    //
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(205.0f, 105.0f)},  //
      {.position = Point(205.0f, 95.0f)},   //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryRoundCaps) {
  auto path =
      PathBuilder().AddLine(Point(100, 100), Point(200, 100)).TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kBevel, Cap::kRound, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(95.3893f, 101.936f)},  //
      {.position = Point(95.3893f, 98.0639f)},  //
      {.position = Point(96.4652f, 103.535f)},  //
      {.position = Point(96.4652f, 96.4652f)},  //
      {.position = Point(98.0639f, 104.611f)},  //
      {.position = Point(98.0639f, 95.3893f)},  //
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(200.0f, 105.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(200.0f, 105.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(201.936f, 104.611f)},  //
      {.position = Point(201.936f, 95.3893f)},  //
      {.position = Point(203.535f, 103.535f)},  //
      {.position = Point(203.535f, 96.4652f)},  //
      {.position = Point(204.611f, 101.936f)},  //
      {.position = Point(204.611f, 98.0639f)},  //
      {.position = Point(205.0f, 100.0f)},      //
      {.position = Point(205.0f, 100.0f)},      //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryBevelJoin) {
  auto path = PathBuilder()
                  .MoveTo(Point(100, 100))
                  .LineTo(Point(200, 100))
                  .LineTo(Point(200, 200))
                  .TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kBevel, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(200.0f, 100.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(205.0f, 100.0f)},  //
      {.position = Point(195.0f, 100.0f)},  //
      {.position = Point(205.0f, 100.0f)},  //
      {.position = Point(195.0f, 200.0f)},  //
      {.position = Point(205.0f, 200.0f)},  //
      {.position = Point(195.0f, 200.0f)},  //
      {.position = Point(205.0f, 200.0f)},  //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryDegenerateBevelJoin) {
  auto path = PathBuilder()
                  .MoveTo(Point(100, 100))
                  .LineTo(Point(200, 100))
                  .LineTo(Point(300, 100))
                  .TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kBevel, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(200.0f, 100.0f)},  //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(300.0f, 105.0f)},  //
      {.position = Point(300.0f, 95.0f)},   //
      {.position = Point(300.0f, 105.0f)},  //
      {.position = Point(300.0f, 95.0f)},   //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryNearlyDegenerateBevelJoin) {
  auto path = PathBuilder()
                  .MoveTo(Point(100, 100))
                  .LineTo(Point(200, 100))
                  .LineTo(Point(300, 107))
                  .TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kBevel, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(200.0f, 105.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(200.0f, 100.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(200.349f, 95.0122f)},  //
      {.position = Point(199.651f, 104.988f)},  //
      {.position = Point(200.349f, 95.0122f)},  //
      {.position = Point(299.651f, 111.988f)},  //
      {.position = Point(300.349f, 102.012f)},  //
      {.position = Point(299.651f, 111.988f)},  //
      {.position = Point(300.349f, 102.012f)},  //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryMiterJoin) {
  auto path = PathBuilder()
                  .MoveTo(Point(100, 100))
                  .LineTo(Point(200, 100))
                  .LineTo(Point(200, 200))
                  .TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kMiter, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(200.0f, 100.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(205.0f, 100.0f)},  //
      {.position = Point(205.0f, 95.0f)},   //
      {.position = Point(195.0f, 100.0f)},  //
      {.position = Point(205.0f, 100.0f)},  //
      {.position = Point(195.0f, 200.0f)},  //
      {.position = Point(205.0f, 200.0f)},  //
      {.position = Point(195.0f, 200.0f)},  //
      {.position = Point(205.0f, 200.0f)},  //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryDegenerateMiterJoin) {
  auto path = PathBuilder()
                  .MoveTo(Point(100, 100))
                  .LineTo(Point(200, 100))
                  .LineTo(Point(300, 100))
                  .TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kMiter, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(300.0f, 105.0f)},  //
      {.position = Point(300.0f, 95.0f)},   //
      {.position = Point(300.0f, 105.0f)},  //
      {.position = Point(300.0f, 95.0f)},   //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryNearlyDegenerateMiterJoin) {
  auto path = PathBuilder()
                  .MoveTo(Point(100, 100))
                  .LineTo(Point(200, 100))
                  .LineTo(Point(300, 107))
                  .TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kMiter, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(200.0f, 105.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(200.0f, 100.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(200.349f, 95.0122f)},  //
      {.position = Point(200.175f, 95.0f)},     //
      {.position = Point(199.651f, 104.988f)},  //
      {.position = Point(200.349f, 95.0122f)},  //
      {.position = Point(299.651f, 111.988f)},  //
      {.position = Point(300.349f, 102.012f)},  //
      {.position = Point(299.651f, 111.988f)},  //
      {.position = Point(300.349f, 102.012f)},  //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryRoundJoin) {
  auto path = PathBuilder()
                  .MoveTo(Point(100, 100))
                  .LineTo(Point(200, 100))
                  .LineTo(Point(200, 200))
                  .TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kRound, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(200.0f, 105.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(200.0f, 100.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(205.0f, 100.0f)},      //
      {.position = Point(201.919f, 95.3664f)},  //
      {.position = Point(204.634f, 98.0807f)},  //
      {.position = Point(203.536f, 96.4645f)},  //
      {.position = Point(203.536f, 96.4645f)},  //
      {.position = Point(195.0f, 100.0f)},      //
      {.position = Point(205.0f, 100.0f)},      //
      {.position = Point(195.0f, 200.0f)},      //
      {.position = Point(205.0f, 200.0f)},      //
      {.position = Point(195.0f, 200.0f)},      //
      {.position = Point(205.0f, 200.0f)},      //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryDegenerateRoundJoin) {
  auto path = PathBuilder()
                  .MoveTo(Point(100, 100))
                  .LineTo(Point(200, 100))
                  .LineTo(Point(300, 100))
                  .TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kRound, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(100.0f, 105.0f)},  //
      {.position = Point(100.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(200.0f, 105.0f)},  //
      {.position = Point(200.0f, 95.0f)},   //
      {.position = Point(300.0f, 105.0f)},  //
      {.position = Point(300.0f, 95.0f)},   //
      {.position = Point(300.0f, 105.0f)},  //
      {.position = Point(300.0f, 95.0f)},   //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

TEST(EntityGeometryTest, StrokePathGeometryNearlyDegenerateRoundJoin) {
  auto path = PathBuilder()
                  .MoveTo(Point(100, 100))
                  .LineTo(Point(200, 100))
                  .LineTo(Point(300, 107))
                  .TakePath();
  auto points = std::make_unique<std::vector<Point>>();
  auto polyline =
      path.CreatePolyline(1.0f, std::move(points),
                          [&points](Path::Polyline::PointBufferPtr reclaimed) {
                            points = std::move(reclaimed);
                          });

  auto vertices = ImpellerEntityUnitTestAccessor::GenerateSolidStrokeVertices(
      polyline, 10.0f, 10.0f, Join::kRound, Cap::kButt, 1.0);

  std::vector<SolidFillVertexShader::PerVertexData> expected = {
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(100.0f, 105.0f)},      //
      {.position = Point(100.0f, 95.0f)},       //
      {.position = Point(200.0f, 105.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(200.0f, 100.0f)},      //
      {.position = Point(200.0f, 95.0f)},       //
      {.position = Point(200.349f, 95.0122f)},  //
      {.position = Point(200.175f, 95.0031f)},  //
      {.position = Point(200.175f, 95.0031f)},  //
      {.position = Point(199.651f, 104.988f)},  //
      {.position = Point(200.349f, 95.0122f)},  //
      {.position = Point(299.651f, 111.988f)},  //
      {.position = Point(300.349f, 102.012f)},  //
      {.position = Point(299.651f, 111.988f)},  //
      {.position = Point(300.349f, 102.012f)},  //
  };

  EXPECT_SOLID_VERTICES_NEAR(vertices, expected);
}

}  // namespace testing
}  // namespace impeller
