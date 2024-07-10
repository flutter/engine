// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/aiks/aiks_unittests.h"

#include "flutter/display_list/dl_blend_mode.h"
#include "flutter/display_list/dl_builder.h"
#include "flutter/display_list/dl_color.h"
#include "flutter/display_list/dl_paint.h"
#include "flutter/display_list/effects/dl_color_filter.h"
#include "flutter/testing/testing.h"
#include "impeller/display_list/dl_image_impeller.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/private/base/SkPoint_impl.h"

namespace impeller {
namespace testing {

using namespace flutter;

TEST_P(AiksTest, RotateColorFilteredPath) {
  DisplayListBuilder builder;
  builder.Transform(SkMatrix::Translate(300, 300) * SkMatrix::RotateDeg(90));

  SkPath arrow_stem;
  SkPath arrow_head;

  arrow_stem.moveTo({120, 190}).lineTo({120, 50});
  arrow_head.moveTo({50, 120}).lineTo({120, 190}).lineTo({190, 120});

  auto filter =
      DlBlendColorFilter::Make(DlColor::kAliceBlue(), DlBlendMode::kSrcIn);

  DlPaint paint;
  paint.setStrokeWidth(15.0);
  paint.setStrokeCap(DlStrokeCap::kRound);
  paint.setStrokeJoin(DlStrokeJoin::kRound);
  paint.setDrawStyle(DlDrawStyle::kStroke);
  paint.setColorFilter(filter);
  paint.setColor(DlColor::kBlack());

  builder.DrawPath(arrow_stem, paint);
  builder.DrawPath(arrow_head, paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderStrokes) {
  DisplayListBuilder builder;
  DlPaint paint;
  paint.setColor(DlColor::kRed());
  paint.setStrokeWidth(20);
  paint.setDrawStyle(DlDrawStyle::kStroke);

  builder.DrawPath(SkPath::Line({200, 100}, {800, 100}), paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderCurvedStrokes) {
  DisplayListBuilder builder;
  DlPaint paint;
  paint.setColor(DlColor::kRed());
  paint.setStrokeWidth(25);
  paint.setDrawStyle(DlDrawStyle::kStroke);

  builder.DrawPath(SkPath::Circle(500, 500, 250), paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderThickCurvedStrokes) {
  DisplayListBuilder builder;
  DlPaint paint;
  paint.setColor(DlColor::kRed());
  paint.setStrokeWidth(100);
  paint.setDrawStyle(DlDrawStyle::kStroke);

  builder.DrawPath(SkPath::Circle(100, 100, 50), paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderThinCurvedStrokes) {
  DisplayListBuilder builder;
  DlPaint paint;
  paint.setColor(DlColor::kRed());
  paint.setStrokeWidth(0.01);
  paint.setDrawStyle(DlDrawStyle::kStroke);

  builder.DrawPath(SkPath::Circle(100, 100, 50), paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderStrokePathThatEndsAtSharpTurn) {
  DisplayListBuilder builder;
  DlPaint paint;
  paint.setColor(DlColor::kRed());
  paint.setStrokeWidth(200);
  paint.setDrawStyle(DlDrawStyle::kStroke);

  SkPath path;
  path.addArc(SkRect::MakeXYWH(100, 100, 200, 200), 0, 90);

  builder.DrawPath(path, paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderStrokePathWithCubicLine) {
  DisplayListBuilder builder;

  DlPaint paint;
  paint.setColor(DlColor::kRed());
  paint.setStrokeWidth(20);
  paint.setDrawStyle(DlDrawStyle::kStroke);

  SkPath path;
  path.moveTo(0, 200);
  path.cubicTo(50, 400, 350, 0, 400, 200);

  builder.DrawPath(path, paint);
  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderQuadraticStrokeWithInstantTurn) {
  DisplayListBuilder builder;

  DlPaint paint;
  paint.setColor(DlColor::kRed());
  paint.setStrokeWidth(50);
  paint.setDrawStyle(DlDrawStyle::kStroke);
  paint.setStrokeCap(DlStrokeCap::kRound);

  // Should draw a diagonal pill shape. If flat on either end, the stroke is
  // rendering wrong.
  SkPath path;
  path.moveTo(250, 250);
  path.quadTo(100, 100, 250, 250);

  builder.DrawPath(path, paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderDifferencePaths) {
  DisplayListBuilder builder;

  DlPaint paint;
  paint.setColor(DlColor::kRed());

  SkPoint radii[4] = {{50, 25}, {25, 50}, {50, 25}, {25, 50}};
  SkPath path;
  SkRRect rrect;
  rrect.setRectRadii(SkRect::MakeXYWH(100, 100, 200, 200), radii);
  path.addRRect(rrect);
  path.addCircle(200, 200, 50);
  path.setFillType(SkPathFillType::kEvenOdd);

  builder.DrawImage(
      DlImageImpeller::Make(CreateTextureForFixture("boston.jpg")), {10, 10},
      {});
  builder.DrawPath(path, paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

// Regression test for https://github.com/flutter/flutter/issues/134816.
//
// It should be possible to draw 3 lines, and not have an implicit close path.
TEST_P(AiksTest, CanDrawAnOpenPath) {
  DisplayListBuilder builder;

  // Starting at (50, 50), draw lines from:
  // 1. (50, height)
  // 2. (width, height)
  // 3. (width, 50)
  PathBuilder builder;
  builder.MoveTo({50, 50});
  builder.LineTo({50, 100});
  builder.LineTo({100, 100});
  builder.LineTo({100, 50});

  Paint paint;
  paint.color = Color::Red();
  paint.style = Paint::Style::kStroke;
  paint.stroke_width = 10;

  canvas.DrawPath(builder.TakePath(), paint);

  ASSERT_TRUE(OpenPlaygroundHere(canvas.EndRecordingAsPicture()));
}

TEST_P(AiksTest, CanDrawAnOpenPathThatIsntARect) {
  DisplayListBuilder builder;

  // Draw a stroked path that is explicitly closed to verify
  // It doesn't become a rectangle.
  PathBuilder builder;
  builder.MoveTo({50, 50});
  builder.LineTo({520, 120});
  builder.LineTo({300, 310});
  builder.LineTo({100, 50});
  builder.Close();

  Paint paint;
  paint.color = Color::Red();
  paint.style = Paint::Style::kStroke;
  paint.stroke_width = 10;

  canvas.DrawPath(builder.TakePath(), paint);

  ASSERT_TRUE(OpenPlaygroundHere(canvas.EndRecordingAsPicture()));
}

TEST_P(AiksTest, SolidStrokesRenderCorrectly) {
  // Compare with https://fiddle.skia.org/c/027392122bec8ac2b5d5de00a4b9bbe2
  auto callback = [&](AiksContext& renderer) -> std::optional<Picture> {
    static Color color = Color::Black().WithAlpha(0.5);
    static float scale = 3;
    static bool add_circle_clip = true;

    if (AiksTest::ImGuiBegin("Controls", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color));
      ImGui::SliderFloat("Scale", &scale, 0, 6);
      ImGui::Checkbox("Circle clip", &add_circle_clip);
      ImGui::End();
    }

    DisplayListBuilder builder;
    canvas.Scale(GetContentScale());
    Paint paint;

    paint.color = Color::White();
    canvas.DrawPaint(paint);

    paint.color = color;
    paint.style = Paint::Style::kStroke;
    paint.stroke_width = 10;

    Path path = PathBuilder{}
                    .MoveTo({20, 20})
                    .QuadraticCurveTo({60, 20}, {60, 60})
                    .Close()
                    .MoveTo({60, 20})
                    .QuadraticCurveTo({60, 60}, {20, 60})
                    .TakePath();

    canvas.Scale(Vector2(scale, scale));

    if (add_circle_clip) {
      static PlaygroundPoint circle_clip_point_a(Point(60, 300), 20,
                                                 Color::Red());
      static PlaygroundPoint circle_clip_point_b(Point(600, 300), 20,
                                                 Color::Red());
      auto [handle_a, handle_b] =
          DrawPlaygroundLine(circle_clip_point_a, circle_clip_point_b);

      auto screen_to_canvas = canvas.GetCurrentTransform().Invert();
      Point point_a = screen_to_canvas * handle_a * GetContentScale();
      Point point_b = screen_to_canvas * handle_b * GetContentScale();

      Point middle = (point_a + point_b) / 2;
      auto radius = point_a.GetDistance(middle);
      canvas.ClipPath(PathBuilder{}.AddCircle(middle, radius).TakePath());
    }

    for (auto join : {Join::kBevel, Join::kRound, Join::kMiter}) {
      paint.stroke_join = join;
      for (auto cap : {Cap::kButt, Cap::kSquare, Cap::kRound}) {
        paint.stroke_cap = cap;
        canvas.DrawPath(path, paint);
        canvas.Translate({80, 0});
      }
      canvas.Translate({-240, 60});
    }

    return canvas.EndRecordingAsPicture();
  };

  ASSERT_TRUE(OpenPlaygroundHere(callback));
}

TEST_P(AiksTest, DrawLinesRenderCorrectly) {
  DisplayListBuilder builder;
  canvas.Scale(GetContentScale());
  Paint paint;
  paint.color = Color::Blue();
  paint.stroke_width = 10;

  auto draw = [&canvas](Paint& paint) {
    for (auto cap : {Cap::kButt, Cap::kSquare, Cap::kRound}) {
      paint.stroke_cap = cap;
      Point origin = {100, 100};
      Point p0 = {50, 0};
      Point p1 = {150, 0};
      canvas.DrawLine({150, 100}, {250, 100}, paint);
      for (int d = 15; d < 90; d += 15) {
        Matrix m = Matrix::MakeRotationZ(Degrees(d));
        canvas.DrawLine(origin + m * p0, origin + m * p1, paint);
      }
      canvas.DrawLine({100, 150}, {100, 250}, paint);
      canvas.DrawCircle({origin}, 35, paint);

      canvas.DrawLine({250, 250}, {250, 250}, paint);

      canvas.Translate({250, 0});
    }
    canvas.Translate({-750, 250});
  };

  std::vector<Color> colors = {
      Color{0x1f / 255.0, 0.0, 0x5c / 255.0, 1.0},
      Color{0x5b / 255.0, 0.0, 0x60 / 255.0, 1.0},
      Color{0x87 / 255.0, 0x01 / 255.0, 0x60 / 255.0, 1.0},
      Color{0xac / 255.0, 0x25 / 255.0, 0x53 / 255.0, 1.0},
      Color{0xe1 / 255.0, 0x6b / 255.0, 0x5c / 255.0, 1.0},
      Color{0xf3 / 255.0, 0x90 / 255.0, 0x60 / 255.0, 1.0},
      Color{0xff / 255.0, 0xb5 / 255.0, 0x6b / 250.0, 1.0}};
  std::vector<Scalar> stops = {
      0.0,
      (1.0 / 6.0) * 1,
      (1.0 / 6.0) * 2,
      (1.0 / 6.0) * 3,
      (1.0 / 6.0) * 4,
      (1.0 / 6.0) * 5,
      1.0,
  };

  auto texture = CreateTextureForFixture("airplane.jpg",
                                         /*enable_mipmapping=*/true);

  draw(paint);

  paint.color_source = ColorSource::MakeRadialGradient(
      {100, 100}, 200, std::move(colors), std::move(stops),
      Entity::TileMode::kMirror, {});
  draw(paint);

  paint.color_source = ColorSource::MakeImage(
      texture, Entity::TileMode::kRepeat, Entity::TileMode::kRepeat, {},
      Matrix::MakeTranslation({-150, 75}));
  draw(paint);

  ASSERT_TRUE(OpenPlaygroundHere(canvas.EndRecordingAsPicture()));
}

TEST_P(AiksTest, DrawRectStrokesRenderCorrectly) {
  DisplayListBuilder builder;
  Paint paint;
  paint.color = Color::Red();
  paint.style = Paint::Style::kStroke;
  paint.stroke_width = 10;

  canvas.Translate({100, 100});
  canvas.DrawPath(
      PathBuilder{}.AddRect(Rect::MakeSize(Size{100, 100})).TakePath(),
      {paint});

  ASSERT_TRUE(OpenPlaygroundHere(canvas.EndRecordingAsPicture()));
}

TEST_P(AiksTest, DrawRectStrokesWithBevelJoinRenderCorrectly) {
  DisplayListBuilder builder;
  Paint paint;
  paint.color = Color::Red();
  paint.style = Paint::Style::kStroke;
  paint.stroke_width = 10;
  paint.stroke_join = Join::kBevel;

  canvas.Translate({100, 100});
  canvas.DrawPath(
      PathBuilder{}.AddRect(Rect::MakeSize(Size{100, 100})).TakePath(),
      {paint});

  ASSERT_TRUE(OpenPlaygroundHere(canvas.EndRecordingAsPicture()));
}

TEST_P(AiksTest, CanDrawMultiContourConvexPath) {
  PathBuilder builder = {};
  for (auto i = 0; i < 10; i++) {
    if (i % 2 == 0) {
      builder.AddCircle(Point(100 + 50 * i, 100 + 50 * i), 100);
    } else {
      builder.MoveTo({100.f + 50.f * i - 100, 100.f + 50.f * i});
      builder.LineTo({100.f + 50.f * i, 100.f + 50.f * i - 100});
      builder.LineTo({100.f + 50.f * i - 100, 100.f + 50.f * i - 100});
      builder.Close();
    }
  }
  builder.SetConvexity(Convexity::kConvex);

  DisplayListBuilder builder;
  canvas.DrawPath(builder.TakePath(), {.color = Color::Red().WithAlpha(0.4)});

  ASSERT_TRUE(OpenPlaygroundHere(canvas.EndRecordingAsPicture()));
}

TEST_P(AiksTest, ArcWithZeroSweepAndBlur) {
  DisplayListBuilder builder;
  canvas.Scale(GetContentScale());

  Paint paint;
  paint.color = Color::Red();
  std::vector<Color> colors = {Color{1.0, 0.0, 0.0, 1.0},
                               Color{0.0, 0.0, 0.0, 1.0}};
  std::vector<Scalar> stops = {0.0, 1.0};
  paint.color_source = ColorSource::MakeSweepGradient(
      {100, 100}, Degrees(45), Degrees(135), std::move(colors),
      std::move(stops), Entity::TileMode::kMirror, {});
  paint.mask_blur_descriptor = Paint::MaskBlurDescriptor{
      .style = FilterContents::BlurStyle::kNormal,
      .sigma = Sigma(20),
  };

  PathBuilder builder;
  builder.AddArc(Rect::MakeXYWH(10, 10, 100, 100), Degrees(0), Degrees(0),
                 false);
  canvas.DrawPath(builder.TakePath(), paint);

  // Check that this empty picture can be created without crashing.
  canvas.EndRecordingAsPicture();
}

TEST_P(AiksTest, CanRenderClips) {
  DisplayListBuilder builder;
  Paint paint;
  paint.color = Color::Fuchsia();
  canvas.ClipPath(
      PathBuilder{}.AddRect(Rect::MakeXYWH(0, 0, 500, 500)).TakePath());
  canvas.DrawPath(PathBuilder{}.AddCircle({500, 500}, 250).TakePath(), paint);
  ASSERT_TRUE(OpenPlaygroundHere(canvas.EndRecordingAsPicture()));
}

TEST_P(AiksTest, CanRenderOverlappingMultiContourPath) {
  DisplayListBuilder builder;

  Paint paint;
  paint.color = Color::Red();

  PathBuilder::RoundingRadii radii;
  radii.top_left = {50, 50};
  radii.top_right = {50, 50};
  radii.bottom_right = {50, 50};
  radii.bottom_left = {50, 50};

  const Scalar kTriangleHeight = 100;
  canvas.Translate(Vector2(200, 200));
  // Form a path similar to the Material drop slider value indicator. Both
  // shapes should render identically side-by-side.
  {
    auto path =
        PathBuilder{}
            .MoveTo({0, kTriangleHeight})
            .LineTo({-kTriangleHeight / 2.0f, 0})
            .LineTo({kTriangleHeight / 2.0f, 0})
            .Close()
            .AddRoundedRect(
                Rect::MakeXYWH(-kTriangleHeight / 2.0f, -kTriangleHeight / 2.0f,
                               kTriangleHeight, kTriangleHeight),
                radii)
            .TakePath();

    canvas.DrawPath(path, paint);
  }
  canvas.Translate(Vector2(100, 0));
  {
    auto path =
        PathBuilder{}
            .MoveTo({0, kTriangleHeight})
            .LineTo({-kTriangleHeight / 2.0f, 0})
            .LineTo({0, -10})
            .LineTo({kTriangleHeight / 2.0f, 0})
            .Close()
            .AddRoundedRect(
                Rect::MakeXYWH(-kTriangleHeight / 2.0f, -kTriangleHeight / 2.0f,
                               kTriangleHeight, kTriangleHeight),
                radii)
            .TakePath();

    canvas.DrawPath(path, paint);
  }

  ASSERT_TRUE(OpenPlaygroundHere(canvas.EndRecordingAsPicture()));
}

}  // namespace testing
}  // namespace impeller
