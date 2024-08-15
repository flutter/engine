// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/aiks/experimental_canvas.h"
#include "impeller/entity/entity_playground.h"
#include "impeller/geometry/path_builder.h"

// TODO(zanderso): https://github.com/flutter/flutter/issues/127701
// NOLINTBEGIN(bugprone-unchecked-optional-access)

namespace impeller {
namespace testing {

using EntityTest = EntityPlayground;

std::unique_ptr<ExperimentalCanvas> CreateTestCanvas(
    EntityTest* test,
    Rect cull_rect = Rect::MakeMaximum()) {
  RenderTarget render_target =
      test->GetContentContext()->GetRenderTargetCache()->CreateOffscreen(
          *test->GetContentContext()->GetContext(), {1, 1}, 1);

  return std::make_unique<ExperimentalCanvas>(*test->GetContentContext(),
                                              render_target, false, cull_rect);
}

TEST_P(EntityTest, EmptyCullRect) {
  auto canvas = CreateTestCanvas(this);

  ASSERT_FALSE(canvas->GetCurrentLocalCullingBounds().has_value());
}

TEST_P(EntityTest, InitialCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), initial_cull);
}

TEST_P(EntityTest, TranslatedCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect translated_cull = Rect::MakeXYWH(0, 0, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->Translate(Vector3(5, 5, 0));

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), translated_cull);
}

TEST_P(EntityTest, ScaledCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect scaled_cull = Rect::MakeXYWH(10, 10, 20, 20);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->Scale(Vector2(0.5, 0.5));

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), scaled_cull);
}

TEST_P(EntityTest, RectClipIntersectAgainstEmptyCullRect) {
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), rect_clip);
}

TEST_P(EntityTest, RectClipDiffAgainstEmptyCullRect) {
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_FALSE(canvas->GetCurrentLocalCullingBounds().has_value());
}

TEST_P(EntityTest, RectClipIntersectAgainstCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(5, 5, 5, 5);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RectClipDiffAgainstNonCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RectClipDiffAboveCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 0, 20, 4);
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RectClipDiffBelowCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 16, 20, 4);
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RectClipDiffLeftOfCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 0, 4, 20);
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RectClipDiffRightOfCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(16, 0, 4, 20);
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RectClipDiffAgainstVCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 0, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 5, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RectClipDiffAgainstHCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 5);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRect(rect_clip, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RRectClipIntersectAgainstEmptyCullRect) {
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this);
  canvas->ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), rect_clip);
}

TEST_P(EntityTest, RRectClipDiffAgainstEmptyCullRect) {
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this);
  canvas->ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_FALSE(canvas->GetCurrentLocalCullingBounds().has_value());
}

TEST_P(EntityTest, RRectClipIntersectAgainstCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(5, 5, 5, 5);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RRectClipDiffAgainstNonCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RRectClipDiffAgainstVPartiallyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, 0, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 6, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RRectClipDiffAgainstVFullyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(5, -2, 10, 14);
  Rect result_cull = Rect::MakeXYWH(0, 0, 5, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RRectClipDiffAgainstHPartiallyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(0, 5, 10, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 6);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, RRectClipDiffAgainstHFullyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  Rect rect_clip = Rect::MakeXYWH(-2, 5, 14, 10);
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 5);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipRRect(rect_clip, {1, 1}, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, PathClipIntersectAgainstEmptyCullRect) {
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(5, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(5, 14, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 14, 1, 1));
  Path path = builder.TakePath();
  Rect rect_clip = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this);
  canvas->ClipPath(path, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), rect_clip);
}

TEST_P(EntityTest, PathClipDiffAgainstEmptyCullRect) {
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(5, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(5, 14, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 14, 1, 1));
  Path path = builder.TakePath();

  auto canvas = CreateTestCanvas(this);
  canvas->ClipPath(path, Entity::ClipOperation::kDifference);

  ASSERT_FALSE(canvas->GetCurrentLocalCullingBounds().has_value());
}

TEST_P(EntityTest, PathClipIntersectAgainstCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(5, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(5, 14, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 14, 1, 1));
  Path path = builder.TakePath();
  Rect result_cull = Rect::MakeXYWH(5, 5, 5, 5);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipPath(path, Entity::ClipOperation::kIntersect);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, PathClipDiffAgainstNonCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(0, 0, 10, 10);
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(5, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(5, 14, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 5, 1, 1));
  builder.AddRect(Rect::MakeXYWH(14, 14, 1, 1));
  Path path = builder.TakePath();
  Rect result_cull = Rect::MakeXYWH(0, 0, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipPath(path, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

TEST_P(EntityTest, PathClipDiffAgainstFullyCoveredCullRect) {
  Rect initial_cull = Rect::MakeXYWH(5, 5, 10, 10);
  PathBuilder builder;
  builder.AddRect(Rect::MakeXYWH(0, 0, 100, 100));
  Path path = builder.TakePath();
  // Diff clip of Paths is ignored due to complexity
  Rect result_cull = Rect::MakeXYWH(5, 5, 10, 10);

  auto canvas = CreateTestCanvas(this, initial_cull);
  canvas->ClipPath(path, Entity::ClipOperation::kDifference);

  ASSERT_TRUE(canvas->GetCurrentLocalCullingBounds().has_value());
  ASSERT_EQ(canvas->GetCurrentLocalCullingBounds().value(), result_cull);
}

}  // namespace testing
}  // namespace impeller

// NOLINTEND(bugprone-unchecked-optional-access)
