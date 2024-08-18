// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/aiks/canvas.h"
#include "impeller/aiks/image_filter.h"
#include "impeller/aiks/save_layer_utils.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/path_builder.h"

// TODO(zanderso): https://github.com/flutter/flutter/issues/127701
// NOLINTBEGIN(bugprone-unchecked-optional-access)

namespace impeller {
namespace testing {

using AiksCanvasTest = ::testing::Test;

TEST(AiksCanvasTest, EmptyCullRect) {
  Canvas canvas;

  ASSERT_FALSE(canvas.GetCurrentLocalCullingBounds().has_value());
}

TEST(AiksCanvasTest, InitialCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);

  Canvas canvas(initial_cull);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), initial_cull);
}

TEST(AiksCanvasTest, TranslatedCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect translated_cull = Rect::MakeXYWH(0, 0, 10, 10);

  Canvas canvas(initial_cull);
  canvas.Translate(Vector3(5, 5, 0));

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), translated_cull);
}

TEST(AiksCanvasTest, ScaledCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect scaled_cull = Rect::MakeXYWH(10, 10, 20, 20);

  Canvas canvas(initial_cull);
  canvas.Scale(Vector2(0.5, 0.5));

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), scaled_cull);
}

TEST(AiksCanvasTest, RectClipIntersectAgainstEmptyCullRect) {
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas;
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), rect_clip);
}

TEST(AiksCanvasTest, RectClipDiffAgainstEmptyCullRect) {
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas;
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_FALSE(canvas.GetCurrentLocalCullingBounds().has_value());
}

TEST(AiksCanvasTest, RectClipIntersectAgainstCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(5, 5, 5, 5);

  Canvas canvas(initial_cull);
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RectClipDiffAgainstNonCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 10);

  Canvas canvas(initial_cull);
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RectClipDiffAboveCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 0, 20, 4);
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas(initial_cull);
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RectClipDiffBelowCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 16, 20, 4);
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas(initial_cull);
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RectClipDiffLeftOfCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 0, 4, 20);
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas(initial_cull);
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RectClipDiffRightOfCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(16, 0, 4, 20);
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas(initial_cull);
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RectClipDiffAgainstVCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 0, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 5, 10);

  Canvas canvas(initial_cull);
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RectClipDiffAgainstHCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 5);

  Canvas canvas(initial_cull);
  canvas.ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RRectClipIntersectAgainstEmptyCullRect) {
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas;
  canvas.ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), rect_clip);
}

TEST(AiksCanvasTest, RRectClipDiffAgainstEmptyCullRect) {
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas;
  canvas.ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_FALSE(canvas.GetCurrentLocalCullingBounds().has_value());
}

TEST(AiksCanvasTest, RRectClipIntersectAgainstCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(5, 5, 5, 5);

  Canvas canvas(initial_cull);
  canvas.ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RRectClipDiffAgainstNonCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 10);

  Canvas canvas(initial_cull);
  canvas.ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RRectClipDiffAgainstVPartiallyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 0, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 6, 10);

  Canvas canvas(initial_cull);
  canvas.ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RRectClipDiffAgainstVFullyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, -2, 10, 14);
  Rect result_cull = Rect::MakeXYWH(0, 0, 5, 10);

  Canvas canvas(initial_cull);
  canvas.ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RRectClipDiffAgainstHPartiallyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 6);

  Canvas canvas(initial_cull);
  canvas.ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, RRectClipDiffAgainstHFullyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(-2, 5, 14, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 5);

  Canvas canvas(initial_cull);
  canvas.ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, PathClipIntersectAgainstEmptyCullRect) {
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(5, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(5, 14, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 14, 1, 1));
  Path path = builder.TakePath();
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas;
  canvas.ClipPath(path, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), rect_clip);
}

TEST(AiksCanvasTest, PathClipDiffAgainstEmptyCullRect) {
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(5, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(5, 14, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 14, 1, 1));
  Path path = builder.TakePath();

  Canvas canvas;
  canvas.ClipPath(path, Entity::ClipOperation::kDifference);

  ASSERT_FALSE(canvas.GetCurrentLocalCullingBounds().has_value());
}

TEST(AiksCanvasTest, PathClipIntersectAgainstCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(5, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(5, 14, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 14, 1, 1));
  Path path = builder.TakePath();
  Rect result_cull = Rect::MakeXYWH(5, 5, 5, 5);

  Canvas canvas(initial_cull);
  canvas.ClipPath(path, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, PathClipDiffAgainstNonCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(5, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(5, 14, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 14, 1, 1));
  Path path = builder.TakePath();
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 10);

  Canvas canvas(initial_cull);
  canvas.ClipPath(path, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, PathClipDiffAgainstFullyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(0, 0, 100, 100));
  Path path = builder.TakePath();
  // Diff clip of Paths is ignored due to complexity
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  Canvas canvas(initial_cull);
  canvas.ClipPath(path, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas.GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST(AiksCanvasTest, DisableLocalBoundsRectForFilteredSaveLayers) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);

  Canvas canvas(initial_cull);
  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());

  canvas.Save();
  canvas.SaveLayer(
      Paint{.image_filter = ImageFilter::MakeBlur(
                Sigma(10), Sigma(10), FilterContents::BlurStyle::kNormal,
                Entity::TileMode::kDecal)});
  ASSERT_FALSE(canvas.GetCurrentLocalCullingBounds().has_value());

  canvas.Restore();
  ASSERT_TRUE(canvas.GetCurrentLocalCullingBounds().has_value());
}

TEST(AiksCanvasTest, ComputeSaveLayerCoverage) {
  // Basic Case, simple paint, computed coverage
  {
    auto coverage =
        ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 10, 10), nullptr, {}, {},
                                 Rect::MakeLTRB(0, 0, 2400, 1800));
    ASSERT_TRUE(coverage.has_value());
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
  }
  // Destructive paint, computed coverage
  {
    auto coverage =
        ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 10, 10), nullptr,
                                 {.blend_mode = BlendMode::kClear}, {},
                                 Rect::MakeLTRB(0, 0, 2400, 1800));
    ASSERT_TRUE(coverage.has_value());
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
  }
  // Backdrop Filter, computed coverage
  {
    auto backdrop_filter =
        ImageFilter::MakeMatrix(Matrix::MakeScale({2, 2, 1}), {});
    auto coverage =
        ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 10, 10), backdrop_filter,
                                 {}, {}, Rect::MakeLTRB(0, 0, 2400, 1800));
    ASSERT_TRUE(coverage.has_value());
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
  }
  // Image Filter, computed coverage
  {
    auto image_filter =
        ImageFilter::MakeMatrix(Matrix::MakeScale({2, 2, 1}), {});
    auto coverage = ComputeSaveLayerCoverage(
        Rect::MakeLTRB(0, 0, 10, 10), nullptr, {.image_filter = image_filter},
        {}, Rect::MakeLTRB(0, 0, 2400, 1800));
    ASSERT_TRUE(coverage.has_value());
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
  }
  // Image Filter scaling large, computed coverage is larger than bounds limit.
  {
    auto image_filter =
        ImageFilter::MakeMatrix(Matrix::MakeScale({2, 2, 1}), {});
    auto coverage = ComputeSaveLayerCoverage(
        Rect::MakeLTRB(0, 0, 10, 10), nullptr, {.image_filter = image_filter},
        {}, Rect::MakeLTRB(0, 0, 5, 5));
    ASSERT_TRUE(coverage.has_value());
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2.5, 2.5));
  }
  // Image Filter scaling small, computed coverage is larger than bounds limit.
  {
    auto image_filter =
        ImageFilter::MakeMatrix(Matrix::MakeScale({0.5, 0.5, 1}), {});
    auto coverage = ComputeSaveLayerCoverage(
        Rect::MakeLTRB(0, 0, 10, 10), nullptr, {.image_filter = image_filter},
        {}, Rect::MakeLTRB(0, 0, 5, 5));
    ASSERT_TRUE(coverage.has_value());
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
  }
  // Coverage disjoint from parent coverage.
  {
    auto coverage =
        ComputeSaveLayerCoverage(Rect::MakeLTRB(200, 200, 210, 210), nullptr,
                                 {}, {}, Rect::MakeLTRB(0, 0, 100, 100));
    EXPECT_FALSE(coverage.has_value());
  }
  // Coverage disjoint from parent coverage but transformed into parent space
  // with image filter.
  {
    auto image_filter =
        ImageFilter::MakeMatrix(Matrix::MakeTranslation({-200, -200, 0}), {});
    auto coverage = ComputeSaveLayerCoverage(
        Rect::MakeLTRB(200, 200, 210, 210), nullptr,
        {.image_filter = image_filter}, {}, Rect::MakeLTRB(0, 0, 100, 100));
    ASSERT_TRUE(coverage.has_value());
    // Is this the right value? should it actually be (0, 0, 10, 10)?
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(200, 200, 210, 210));
  }
  // Coverage disjoint from parent coverage. CTM does not impact lack of
  // intersection as it has already been "absorbed" by child coverage.
  {
    Matrix ctm = Matrix::MakeTranslation({-200, -200, 0});
    auto coverage =
        ComputeSaveLayerCoverage(Rect::MakeLTRB(200, 200, 210, 210), nullptr,
                                 {}, ctm, Rect::MakeLTRB(0, 0, 100, 100));
    ASSERT_FALSE(coverage.has_value());
  }
  // Empty coverage.
  {
    auto coverage =
        ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 0, 0), nullptr, {}, {},
                                 Rect::MakeLTRB(0, 0, 2400, 1800));
    ASSERT_FALSE(coverage.has_value());
  }
  // Empty coverage with Image Filter
  {
    auto image_filter =
        ImageFilter::MakeMatrix(Matrix::MakeTranslation({-200, -200, 0}), {});
    auto coverage = ComputeSaveLayerCoverage(
        Rect::MakeLTRB(0, 0, 0, 0), nullptr, {.image_filter = image_filter}, {},
        Rect::MakeLTRB(0, 0, 2400, 1800));
    ASSERT_FALSE(coverage.has_value());
  }
  // Empty coverage with backdrop filter.
  {
    auto image_filter =
        ImageFilter::MakeMatrix(Matrix::MakeTranslation({-200, -200, 0}), {});
    auto coverage =
        ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 0, 0), image_filter, {},
                                 {}, Rect::MakeLTRB(0, 0, 2400, 1800));
    ASSERT_TRUE(coverage.has_value());
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
  }
  // Destructive paint, user specified bounds.
  {
    auto coverage = ComputeSaveLayerCoverage(
        Rect::MakeLTRB(0, 0, 10, 10), nullptr,
        {.blend_mode = BlendMode::kClear}, {}, Rect::MakeLTRB(0, 0, 2400, 1800),
        /*user_provided_bounds=*/true);
    ASSERT_TRUE(coverage.has_value());
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
  }
  // Backdrop Filter, user specified bounds.
  {
    auto backdrop_filter =
        ImageFilter::MakeMatrix(Matrix::MakeScale({2, 2, 1}), {});
    auto coverage = ComputeSaveLayerCoverage(
        Rect::MakeLTRB(0, 0, 10, 10), backdrop_filter, {}, {},
        Rect::MakeLTRB(0, 0, 2400, 1800), /*user_provided_bounds=*/true);
    ASSERT_TRUE(coverage.has_value());
    EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
  }
}

}  // namespace testing
}  // namespace impeller

// NOLINTEND(bugprone-unchecked-optional-access)
