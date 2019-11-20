// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_rect_layer.h"

#include "flutter/flow/testing/layer_test.h"
#include "flutter/flow/testing/mock_layer.h"
#include "flutter/fml/macros.h"
#include "flutter/testing/mock_canvas.h"

namespace flutter {
namespace testing {

static constexpr SkRect kEmptyRect = SkRect::MakeEmpty();

using ClipRectLayerDeathTest = LayerTest;
using ClipRectLayerTest = LayerTest;

TEST_F(ClipRectLayerDeathTest, ClipNone) {
  EXPECT_DEATH_IF_SUPPORTED(
      std::make_shared<ClipRectLayer>(kEmptyRect, Clip::none),
      "clip_behavior != Clip::none");
}

TEST_F(ClipRectLayerDeathTest, EmptyLayer) {
  auto layer = std::make_shared<ClipRectLayer>(kEmptyRect, Clip::hardEdge);

  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(preroll_context()->cull_rect, kGiantRect);     // Leaves untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.empty());  // Leaves untouched
  EXPECT_EQ(layer->paint_bounds(), kEmptyRect);
  EXPECT_FALSE(layer->needs_painting());

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}

TEST_F(ClipRectLayerDeathTest, PaintBeforePreroll) {
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  auto layer = std::make_shared<ClipRectLayer>(layer_bounds, Clip::hardEdge);
  EXPECT_EQ(layer->paint_bounds(), kEmptyRect);
  EXPECT_FALSE(layer->needs_painting());

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}

TEST_F(ClipRectLayerDeathTest, CulledLayer) {
  const SkMatrix initial_matrix = SkMatrix::MakeTrans(0.5f, 1.0f);
  const SkRect child_bounds = SkRect::MakeXYWH(1.0, 2.0, 2.0, 2.0);
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath child_path = SkPath().addRect(child_bounds);
  auto mock_layer = std::make_shared<MockLayer>(child_path);
  auto layer = std::make_shared<ClipRectLayer>(layer_bounds, Clip::hardEdge);
  layer->Add(mock_layer);

  preroll_context()->cull_rect = kEmptyRect;  // Cull everything

  layer->Preroll(preroll_context(), initial_matrix);
  EXPECT_EQ(preroll_context()->cull_rect, kEmptyRect);     // Leaves untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.empty());  // Leaves untouched
  EXPECT_EQ(mock_layer->paint_bounds(), kEmptyRect);
  EXPECT_EQ(layer->paint_bounds(), kEmptyRect);
  EXPECT_FALSE(mock_layer->needs_painting());
  EXPECT_FALSE(layer->needs_painting());
  mock_layer->ExpectParentCullRect(kEmptyRect);
  mock_layer->ExpectParentMatrix(SkMatrix());
  mock_layer->ExpectMutators({});

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}

TEST_F(ClipRectLayerTest, ChildOutsideBounds) {
  const SkMatrix initial_matrix = SkMatrix::MakeTrans(0.5f, 1.0f);
  const SkRect cull_bounds = SkRect::MakeXYWH(0.0, 0.0, 2.0, 4.0);
  const SkRect child_bounds = SkRect::MakeXYWH(2.5, 5.0, 4.5, 4.0);
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath child_path = SkPath().addRect(child_bounds);
  const SkPaint child_paint = SkPaint(SkColors::kYellow);
  auto mock_layer = std::make_shared<MockLayer>(child_path, child_paint);
  auto layer = std::make_shared<ClipRectLayer>(layer_bounds, Clip::hardEdge);
  layer->Add(mock_layer);

  SkRect intersect_bounds = layer_bounds;
  SkRect child_intersect_bounds = layer_bounds;
  intersect_bounds.intersect(cull_bounds);
  child_intersect_bounds.intersect(child_bounds);
  preroll_context()->cull_rect = cull_bounds;  // Cull child

  layer->Preroll(preroll_context(), initial_matrix);
  EXPECT_EQ(preroll_context()->cull_rect, cull_bounds);    // Leaves untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.empty());  // Leaves untouched
  EXPECT_EQ(mock_layer->paint_bounds(), child_bounds);
  EXPECT_EQ(layer->paint_bounds(), child_intersect_bounds);
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer->needs_painting());
  mock_layer->ExpectParentCullRect(intersect_bounds);
  mock_layer->ExpectParentMatrix(initial_matrix);
  mock_layer->ExpectMutators({Mutator(layer_bounds)});

  // TODO(dworsham):  This seems like a bug.  This should be a death test.
  // Even though `context->cull_rect` and `child_bounds` are disjoint, `layer`
  // and `mock_layer` still get drawn.
  //
  // Should we do
  //  `context->cull_rect.intersect(child_paint_bounds);
  //   set_paint_bounds(context->cull_rect);`
  // inside of |ClipRectLayer| instead?
  layer->Paint(paint_context());
  mock_canvas().ExpectDrawCalls(
      {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
       MockCanvas::DrawCall{
           1, MockCanvas::ClipRectData{layer_bounds, SkClipOp::kIntersect,
                                       MockCanvas::kHard_ClipEdgeStyle}},
       MockCanvas::DrawCall{1,
                            MockCanvas::DrawPathData{child_path, child_paint}},
       MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}});
}

TEST_F(ClipRectLayerTest, FullyContainedChild) {
  const SkMatrix initial_matrix = SkMatrix::MakeTrans(0.5f, 1.0f);
  const SkRect child_bounds = SkRect::MakeXYWH(1.0, 2.0, 2.0, 2.0);
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath child_path = SkPath().addRect(child_bounds);
  const SkPaint child_paint = SkPaint(SkColors::kYellow);
  auto mock_layer = std::make_shared<MockLayer>(child_path, child_paint);
  auto layer = std::make_shared<ClipRectLayer>(layer_bounds, Clip::hardEdge);
  layer->Add(mock_layer);

  layer->Preroll(preroll_context(), initial_matrix);
  EXPECT_EQ(preroll_context()->cull_rect, kGiantRect);     // Leaves untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.empty());  // Leaves untouched
  EXPECT_EQ(mock_layer->paint_bounds(), child_bounds);
  EXPECT_EQ(layer->paint_bounds(), mock_layer->paint_bounds());
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer->needs_painting());
  mock_layer->ExpectParentCullRect(layer_bounds);
  mock_layer->ExpectParentMatrix(initial_matrix);
  mock_layer->ExpectMutators({Mutator(layer_bounds)});

  layer->Paint(paint_context());
  mock_canvas().ExpectDrawCalls(
      {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
       MockCanvas::DrawCall{
           1, MockCanvas::ClipRectData{layer_bounds, SkClipOp::kIntersect,
                                       MockCanvas::kHard_ClipEdgeStyle}},
       MockCanvas::DrawCall{1,
                            MockCanvas::DrawPathData{child_path, child_paint}},
       MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}});
}

TEST_F(ClipRectLayerTest, PartiallyContainedChild) {
  const SkMatrix initial_matrix = SkMatrix::MakeTrans(0.5f, 1.0f);
  const SkRect cull_bounds = SkRect::MakeXYWH(0.0, 0.0, 4.0, 5.5);
  const SkRect child_bounds = SkRect::MakeXYWH(2.5, 5.0, 4.5, 4.0);
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath child_path = SkPath().addRect(child_bounds);
  const SkPaint child_paint = SkPaint(SkColors::kYellow);
  auto mock_layer = std::make_shared<MockLayer>(child_path, child_paint);
  auto layer = std::make_shared<ClipRectLayer>(layer_bounds, Clip::hardEdge);
  layer->Add(mock_layer);

  SkRect intersect_bounds = layer_bounds;
  SkRect child_intersect_bounds = layer_bounds;
  intersect_bounds.intersect(cull_bounds);
  child_intersect_bounds.intersect(child_bounds);
  preroll_context()->cull_rect = cull_bounds;  // Cull child

  layer->Preroll(preroll_context(), initial_matrix);
  EXPECT_EQ(preroll_context()->cull_rect, cull_bounds);    // Leaves untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.empty());  // Leaves untouched
  EXPECT_EQ(mock_layer->paint_bounds(), child_bounds);
  // TODO(dworsham):  This seems like a bug.  It doesn't take
  // `context->cull_rect` into account at all.
  //
  // Should we do
  //  `context->cull_rect.intersect(child_paint_bounds);
  //   set_paint_bounds(context->cull_rect);`
  // inside of |ClipRectLayer| instead?
  EXPECT_EQ(layer->paint_bounds(), child_intersect_bounds);
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer->needs_painting());
  mock_layer->ExpectParentCullRect(intersect_bounds);
  mock_layer->ExpectParentMatrix(initial_matrix);
  mock_layer->ExpectMutators({Mutator(layer_bounds)});

  layer->Paint(paint_context());
  mock_canvas().ExpectDrawCalls(
      {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
       MockCanvas::DrawCall{
           1, MockCanvas::ClipRectData{layer_bounds, SkClipOp::kIntersect,
                                       MockCanvas::kHard_ClipEdgeStyle}},
       MockCanvas::DrawCall{1,
                            MockCanvas::DrawPathData{child_path, child_paint}},
       MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}});
}

}  // namespace testing
}  // namespace flutter
