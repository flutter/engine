// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_path_layer.h"

#include "flutter/flow/testing/layer_test.h"
#include "flutter/flow/testing/mock_layer.h"
#include "flutter/fml/macros.h"
#include "flutter/testing/mock_canvas.h"

namespace flutter {
namespace testing {

using ClipPathLayerTest = LayerTest;

#ifndef NDEBUG
TEST_F(ClipPathLayerTest, ClipNoneBehaviorDies) {
  EXPECT_DEATH_IF_SUPPORTED(
      auto clip = std::make_shared<ClipPathLayer>(SkPath(), Clip::none),
      "clip_behavior != Clip::none");
}

TEST_F(ClipPathLayerTest, PaintingEmptyLayerDies) {
  auto layer = std::make_shared<ClipPathLayer>(SkPath(), Clip::hardEdge);

  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(preroll_context()->cull_rect, kGiantRect);        // Untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.is_empty());  // Untouched
  EXPECT_EQ(layer->paint_bounds(), kEmptyRect);
  EXPECT_FALSE(layer->needs_painting());

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}

TEST_F(ClipPathLayerTest, PaintBeforePrerollDies) {
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath layer_path = SkPath().addRect(layer_bounds);
  auto layer = std::make_shared<ClipPathLayer>(layer_path, Clip::hardEdge);
  EXPECT_EQ(layer->paint_bounds(), kEmptyRect);
  EXPECT_FALSE(layer->needs_painting());

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}

TEST_F(ClipPathLayerTest, PaintingCulledLayerDies) {
  const SkMatrix initial_matrix = SkMatrix::MakeTrans(0.5f, 1.0f);
  const SkRect child_bounds = SkRect::MakeXYWH(1.0, 2.0, 2.0, 2.0);
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath child_path = SkPath().addRect(child_bounds);
  const SkPath layer_path = SkPath().addRect(layer_bounds);
  auto mock_layer = std::make_shared<MockLayer>(child_path);
  auto layer = std::make_shared<ClipPathLayer>(layer_path, Clip::hardEdge);
  layer->Add(mock_layer);

  preroll_context()->cull_rect = kEmptyRect;  // Cull everything

  layer->Preroll(preroll_context(), initial_matrix);
  EXPECT_EQ(preroll_context()->cull_rect, kEmptyRect);        // Untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.is_empty());  // Untouched
  EXPECT_EQ(mock_layer->paint_bounds(), kEmptyRect);
  EXPECT_EQ(layer->paint_bounds(), kEmptyRect);
  EXPECT_FALSE(mock_layer->needs_painting());
  EXPECT_FALSE(layer->needs_painting());
  EXPECT_EQ(mock_layer->parent_cull_rect(), kEmptyRect);
  EXPECT_EQ(mock_layer->parent_matrix(), SkMatrix());
  EXPECT_EQ(mock_layer->parent_mutators(), std::vector<Mutator>());

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}
#endif

TEST_F(ClipPathLayerTest, ChildOutsideBounds) {
  const SkMatrix initial_matrix = SkMatrix::MakeTrans(0.5f, 1.0f);
  const SkRect cull_bounds = SkRect::MakeXYWH(0.0, 0.0, 2.0, 4.0);
  const SkRect child_bounds = SkRect::MakeXYWH(2.5, 5.0, 4.5, 4.0);
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath child_path = SkPath().addRect(child_bounds);
  const SkPath layer_path = SkPath().addRect(layer_bounds);
  const SkPaint child_paint = SkPaint(SkColors::kYellow);
  auto mock_layer = std::make_shared<MockLayer>(child_path, child_paint);
  auto layer = std::make_shared<ClipPathLayer>(layer_path, Clip::hardEdge);
  layer->Add(mock_layer);

  SkRect intersect_bounds = layer_bounds;
  SkRect child_intersect_bounds = layer_bounds;
  intersect_bounds.intersect(cull_bounds);
  child_intersect_bounds.intersect(child_bounds);
  preroll_context()->cull_rect = cull_bounds;  // Cull child

  layer->Preroll(preroll_context(), initial_matrix);
  EXPECT_EQ(preroll_context()->cull_rect, cull_bounds);       // Untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.is_empty());  // Untouched
  EXPECT_EQ(mock_layer->paint_bounds(), child_bounds);
  EXPECT_EQ(layer->paint_bounds(), child_intersect_bounds);
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer->needs_painting());
  EXPECT_EQ(mock_layer->parent_cull_rect(), intersect_bounds);
  EXPECT_EQ(mock_layer->parent_matrix(), initial_matrix);
  EXPECT_EQ(mock_layer->parent_mutators(), std::vector({Mutator(layer_path)}));

  layer->Paint(paint_context());
  EXPECT_EQ(
      mock_canvas().draw_calls(),
      std::vector(
          {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
           MockCanvas::DrawCall{
               1, MockCanvas::ClipRectData{layer_bounds, SkClipOp::kIntersect,
                                           MockCanvas::kHard_ClipEdgeStyle}},
           MockCanvas::DrawCall{
               1, MockCanvas::DrawPathData{child_path, child_paint}},
           MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}}));
}

TEST_F(ClipPathLayerTest, FullyContainedChild) {
  const SkMatrix initial_matrix = SkMatrix::MakeTrans(0.5f, 1.0f);
  const SkRect child_bounds = SkRect::MakeXYWH(1.0, 2.0, 2.0, 2.0);
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath child_path = SkPath().addRect(child_bounds);
  const SkPath layer_path = SkPath().addRect(layer_bounds);
  const SkPaint child_paint = SkPaint(SkColors::kYellow);
  auto mock_layer = std::make_shared<MockLayer>(child_path, child_paint);
  auto layer = std::make_shared<ClipPathLayer>(layer_path, Clip::hardEdge);
  layer->Add(mock_layer);

  layer->Preroll(preroll_context(), initial_matrix);
  EXPECT_EQ(preroll_context()->cull_rect, kGiantRect);        // Untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.is_empty());  // Untouched
  EXPECT_EQ(mock_layer->paint_bounds(), child_bounds);
  EXPECT_EQ(layer->paint_bounds(), mock_layer->paint_bounds());
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer->needs_painting());
  EXPECT_EQ(mock_layer->parent_cull_rect(), layer_bounds);
  EXPECT_EQ(mock_layer->parent_matrix(), initial_matrix);
  EXPECT_EQ(mock_layer->parent_mutators(), std::vector({Mutator(layer_path)}));

  layer->Paint(paint_context());
  EXPECT_EQ(
      mock_canvas().draw_calls(),
      std::vector(
          {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
           MockCanvas::DrawCall{
               1, MockCanvas::ClipRectData{layer_bounds, SkClipOp::kIntersect,
                                           MockCanvas::kHard_ClipEdgeStyle}},
           MockCanvas::DrawCall{
               1, MockCanvas::DrawPathData{child_path, child_paint}},
           MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}}));
}

TEST_F(ClipPathLayerTest, PartiallyContainedChild) {
  const SkMatrix initial_matrix = SkMatrix::MakeTrans(0.5f, 1.0f);
  const SkRect cull_bounds = SkRect::MakeXYWH(0.0, 0.0, 4.0, 5.5);
  const SkRect child_bounds = SkRect::MakeXYWH(2.5, 5.0, 4.5, 4.0);
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath child_path = SkPath().addRect(child_bounds);
  const SkPath layer_path = SkPath().addRect(layer_bounds);
  const SkPaint child_paint = SkPaint(SkColors::kYellow);
  auto mock_layer = std::make_shared<MockLayer>(child_path, child_paint);
  auto layer = std::make_shared<ClipPathLayer>(layer_path, Clip::hardEdge);
  layer->Add(mock_layer);

  SkRect intersect_bounds = layer_bounds;
  SkRect child_intersect_bounds = layer_bounds;
  intersect_bounds.intersect(cull_bounds);
  child_intersect_bounds.intersect(child_bounds);
  preroll_context()->cull_rect = cull_bounds;  // Cull child

  layer->Preroll(preroll_context(), initial_matrix);
  EXPECT_EQ(preroll_context()->cull_rect, cull_bounds);       // Untouched
  EXPECT_TRUE(preroll_context()->mutators_stack.is_empty());  // Untouched
  EXPECT_EQ(mock_layer->paint_bounds(), child_bounds);
  EXPECT_EQ(layer->paint_bounds(), child_intersect_bounds);
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer->needs_painting());
  EXPECT_EQ(mock_layer->parent_cull_rect(), intersect_bounds);
  EXPECT_EQ(mock_layer->parent_matrix(), initial_matrix);
  EXPECT_EQ(mock_layer->parent_mutators(), std::vector({Mutator(layer_path)}));

  layer->Paint(paint_context());
  EXPECT_EQ(
      mock_canvas().draw_calls(),
      std::vector(
          {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
           MockCanvas::DrawCall{
               1, MockCanvas::ClipRectData{layer_bounds, SkClipOp::kIntersect,
                                           MockCanvas::kHard_ClipEdgeStyle}},
           MockCanvas::DrawCall{
               1, MockCanvas::DrawPathData{child_path, child_paint}},
           MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}}));
}

#define CHECK_READBACK(layer, before, after)                       \
  do {                                                             \
    preroll_context()->layer_reads_from_surface = before;          \
    layer->Preroll(preroll_context(), initial_matrix);             \
    EXPECT_EQ(preroll_context()->layer_reads_from_surface, after); \
  } while (0)

#define CLIP_RECT_CHECK_READBACK(clip, before, after)               \
  do {                                                              \
    auto layer = std::make_shared<ClipPathLayer>(layer_path, clip); \
    CHECK_READBACK(layer, before, after);                           \
  } while (0)

#define CLIP_RECT_CHILD_CHECK_READBACK(clip, child, before, after)  \
  do {                                                              \
    auto layer = std::make_shared<ClipPathLayer>(layer_path, clip); \
    layer->Add(child);                                              \
    CHECK_READBACK(layer, before, after);                           \
  } while (0)

TEST_F(ClipPathLayerTest, Readback) {
  const SkMatrix initial_matrix = SkMatrix();
  const SkRect layer_bounds = SkRect::MakeXYWH(0.5, 1.0, 5.0, 6.0);
  const SkPath layer_path = SkPath().addRect(layer_bounds);

  const Clip hard = Clip::hardEdge;
  const Clip soft = Clip::antiAlias;
  const Clip save_layer = Clip::antiAliasWithSaveLayer;

  auto mock_read_layer =
      std::make_shared<MockLayer>(SkPath(), SkPaint(), false, false, true);
  auto mock_no_read_layer = std::make_shared<MockLayer>(SkPath(), SkPaint());

  // No children, no prior readback -> no readback after
  CLIP_RECT_CHECK_READBACK(hard, false, false);
  CLIP_RECT_CHECK_READBACK(soft, false, false);
  CLIP_RECT_CHECK_READBACK(save_layer, false, false);

  // No children, prior readback -> readback after
  CLIP_RECT_CHECK_READBACK(hard, true, true);
  CLIP_RECT_CHECK_READBACK(soft, true, true);
  CLIP_RECT_CHECK_READBACK(save_layer, true, true);

  // Non readback child, no prior readback -> no readback after
  CLIP_RECT_CHILD_CHECK_READBACK(hard, mock_no_read_layer, false, false);
  CLIP_RECT_CHILD_CHECK_READBACK(soft, mock_no_read_layer, false, false);
  CLIP_RECT_CHILD_CHECK_READBACK(save_layer, mock_no_read_layer, false, false);

  // Non readback child, prior readback -> readback after
  CLIP_RECT_CHILD_CHECK_READBACK(hard, mock_no_read_layer, true, true);
  CLIP_RECT_CHILD_CHECK_READBACK(soft, mock_no_read_layer, true, true);
  CLIP_RECT_CHILD_CHECK_READBACK(save_layer, mock_no_read_layer, true, true);

  // Readback child, no prior readback -> readback after unless SaveLayer
  CLIP_RECT_CHILD_CHECK_READBACK(hard, mock_read_layer, false, true);
  CLIP_RECT_CHILD_CHECK_READBACK(soft, mock_read_layer, false, true);
  CLIP_RECT_CHILD_CHECK_READBACK(save_layer, mock_read_layer, false, false);

  // Readback child, prior readback -> readback after
  CLIP_RECT_CHILD_CHECK_READBACK(hard, mock_read_layer, true, true);
  CLIP_RECT_CHILD_CHECK_READBACK(soft, mock_read_layer, true, true);
  CLIP_RECT_CHILD_CHECK_READBACK(save_layer, mock_read_layer, true, true);
}

}  // namespace testing
}  // namespace flutter
