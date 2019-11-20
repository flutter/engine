// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/transform_layer.h"

#include "flutter/flow/testing/layer_test.h"
#include "flutter/flow/testing/mock_layer.h"
#include "flutter/fml/macros.h"
#include "flutter/testing/mock_canvas.h"

namespace flutter {
namespace testing {

using TransformLayerDeathTest = LayerTest;
using TransformLayerTest = LayerTest;

TEST_F(TransformLayerDeathTest, EmptyLayer) {
  auto layer = std::make_shared<TransformLayer>(SkMatrix());  // identity

  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(layer->paint_bounds(), SkRect::MakeEmpty());
  EXPECT_FALSE(layer->needs_painting());

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}

TEST_F(TransformLayerDeathTest, PaintBeforePreroll) {
  SkPath child_path;
  child_path.addRect(5.0f, 6.0f, 20.5f, 21.5f);
  auto mock_layer = std::make_shared<MockLayer>(child_path, SkPaint());
  auto layer = std::make_shared<TransformLayer>(SkMatrix());  // identity
  layer->Add(mock_layer);

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}

TEST_F(TransformLayerTest, Identity) {
  SkPath child_path;
  child_path.addRect(5.0f, 6.0f, 20.5f, 21.5f);
  SkRect cull_rect = SkRect::MakeXYWH(2.0f, 2.0f, 14.0f, 14.0f);
  auto mock_layer = std::make_shared<MockLayer>(child_path, SkPaint());
  auto layer = std::make_shared<TransformLayer>(SkMatrix());  // identity
  layer->Add(mock_layer);

  preroll_context()->cull_rect = cull_rect;
  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(mock_layer->paint_bounds(), child_path.getBounds());
  EXPECT_EQ(layer->paint_bounds(), mock_layer->paint_bounds());
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer->needs_painting());
  mock_layer->ExpectParentMatrix(SkMatrix());  // identity
  mock_layer->ExpectParentCullRect(cull_rect);
  mock_layer->ExpectMutators({Mutator(SkMatrix())});

  layer->Paint(paint_context());
  mock_canvas().ExpectDrawCalls({MockCanvas::DrawCall{
      0, MockCanvas::DrawPathData{child_path, SkPaint()}}});
}

TEST_F(TransformLayerTest, Simple) {
  SkPath child_path;
  child_path.addRect(5.0f, 6.0f, 20.5f, 21.5f);
  SkRect cull_rect = SkRect::MakeXYWH(2.0f, 2.0f, 14.0f, 14.0f);
  SkMatrix initial_transform = SkMatrix::MakeTrans(-0.5f, -0.5f);
  SkMatrix layer_transform = SkMatrix::MakeTrans(2.5f, 2.5f);
  SkMatrix inverse_layer_transform;
  EXPECT_TRUE(layer_transform.invert(&inverse_layer_transform));

  auto mock_layer = std::make_shared<MockLayer>(child_path, SkPaint());
  auto layer = std::make_shared<TransformLayer>(layer_transform);
  layer->Add(mock_layer);

  preroll_context()->cull_rect = cull_rect;
  layer->Preroll(preroll_context(), initial_transform);
  EXPECT_EQ(mock_layer->paint_bounds(), child_path.getBounds());
  EXPECT_EQ(layer->paint_bounds(),
            layer_transform.mapRect(mock_layer->paint_bounds()));
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer->needs_painting());
  mock_layer->ExpectParentMatrix(
      SkMatrix::Concat(initial_transform, layer_transform));
  mock_layer->ExpectParentCullRect(inverse_layer_transform.mapRect(cull_rect));
  mock_layer->ExpectMutators({Mutator(layer_transform)});

  layer->Paint(paint_context());
  mock_canvas().ExpectDrawCalls(
      {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
       MockCanvas::DrawCall{1, MockCanvas::ConcatMatrixData{layer_transform}},
       MockCanvas::DrawCall{1, MockCanvas::DrawPathData{child_path, SkPaint()}},
       MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}});
}

TEST_F(TransformLayerTest, Nested) {
  SkPath child_path;
  child_path.addRect(5.0f, 6.0f, 20.5f, 21.5f);
  SkRect cull_rect = SkRect::MakeXYWH(2.0f, 2.0f, 14.0f, 14.0f);
  SkMatrix initial_transform = SkMatrix::MakeTrans(-0.5f, -0.5f);
  SkMatrix layer1_transform = SkMatrix::MakeTrans(2.5f, 2.5f);
  SkMatrix layer2_transform = SkMatrix::MakeTrans(2.5f, 2.5f);
  SkMatrix inverse_layer1_transform, inverse_layer2_transform;
  EXPECT_TRUE(layer1_transform.invert(&inverse_layer1_transform));
  EXPECT_TRUE(layer2_transform.invert(&inverse_layer2_transform));

  auto mock_layer = std::make_shared<MockLayer>(child_path, SkPaint());
  auto layer1 = std::make_shared<TransformLayer>(layer1_transform);
  auto layer2 = std::make_shared<TransformLayer>(layer2_transform);
  layer1->Add(layer2);
  layer2->Add(mock_layer);

  preroll_context()->cull_rect = cull_rect;
  layer1->Preroll(preroll_context(), initial_transform);
  EXPECT_EQ(mock_layer->paint_bounds(), child_path.getBounds());
  EXPECT_EQ(layer2->paint_bounds(),
            layer2_transform.mapRect(mock_layer->paint_bounds()));
  EXPECT_EQ(layer1->paint_bounds(),
            layer1_transform.mapRect(layer2->paint_bounds()));
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer2->needs_painting());
  EXPECT_TRUE(layer1->needs_painting());
  mock_layer->ExpectParentMatrix(SkMatrix::Concat(
      SkMatrix::Concat(initial_transform, layer1_transform), layer2_transform));
  mock_layer->ExpectParentCullRect(inverse_layer2_transform.mapRect(
      inverse_layer1_transform.mapRect(cull_rect)));
  mock_layer->ExpectMutators(
      {Mutator(layer2_transform), Mutator(layer1_transform)});

  layer1->Paint(paint_context());
  mock_canvas().ExpectDrawCalls(
      {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
       MockCanvas::DrawCall{1, MockCanvas::ConcatMatrixData{layer1_transform}},
       MockCanvas::DrawCall{1, MockCanvas::SaveData{2}},
       MockCanvas::DrawCall{2, MockCanvas::ConcatMatrixData{layer2_transform}},
       MockCanvas::DrawCall{2, MockCanvas::DrawPathData{child_path, SkPaint()}},
       MockCanvas::DrawCall{2, MockCanvas::RestoreData{1}},
       MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}});
}

TEST_F(TransformLayerTest, NestedSeparated) {
  SkPath child_path;
  child_path.addRect(5.0f, 6.0f, 20.5f, 21.5f);
  SkRect cull_rect = SkRect::MakeXYWH(2.0f, 2.0f, 14.0f, 14.0f);
  SkMatrix initial_transform = SkMatrix::MakeTrans(-0.5f, -0.5f);
  SkMatrix layer1_transform = SkMatrix::MakeTrans(2.5f, 2.5f);
  SkMatrix layer2_transform = SkMatrix::MakeTrans(2.5f, 2.5f);
  SkMatrix inverse_layer1_transform, inverse_layer2_transform;
  EXPECT_TRUE(layer1_transform.invert(&inverse_layer1_transform));
  EXPECT_TRUE(layer2_transform.invert(&inverse_layer2_transform));

  auto mock_layer1 =
      std::make_shared<MockLayer>(child_path, SkPaint(SkColors::kBlue));
  auto mock_layer2 =
      std::make_shared<MockLayer>(child_path, SkPaint(SkColors::kGreen));
  auto layer1 = std::make_shared<TransformLayer>(layer1_transform);
  auto layer2 = std::make_shared<TransformLayer>(layer2_transform);
  layer1->Add(mock_layer1);
  layer2->Add(mock_layer2);
  mock_layer1->SetChild(layer2);

  SkRect expected_mock1_bounds = child_path.getBounds();
  preroll_context()->cull_rect = cull_rect;
  layer1->Preroll(preroll_context(), initial_transform);
  expected_mock1_bounds.join(layer2->paint_bounds());

  EXPECT_EQ(mock_layer2->paint_bounds(), child_path.getBounds());
  EXPECT_EQ(layer2->paint_bounds(),
            layer2_transform.mapRect(mock_layer2->paint_bounds()));
  EXPECT_EQ(mock_layer1->paint_bounds(), expected_mock1_bounds);
  EXPECT_EQ(layer1->paint_bounds(),
            layer1_transform.mapRect(mock_layer1->paint_bounds()));
  EXPECT_TRUE(mock_layer2->needs_painting());
  EXPECT_TRUE(layer2->needs_painting());
  EXPECT_TRUE(mock_layer1->needs_painting());
  EXPECT_TRUE(layer1->needs_painting());
  mock_layer1->ExpectParentMatrix(
      SkMatrix::Concat(initial_transform, layer1_transform));
  mock_layer2->ExpectParentMatrix(SkMatrix::Concat(
      SkMatrix::Concat(initial_transform, layer1_transform), layer2_transform));
  mock_layer1->ExpectParentCullRect(
      inverse_layer1_transform.mapRect(cull_rect));
  mock_layer2->ExpectParentCullRect(inverse_layer2_transform.mapRect(
      inverse_layer1_transform.mapRect(cull_rect)));
  mock_layer1->ExpectMutators({Mutator(layer1_transform)});
  mock_layer2->ExpectMutators(
      {Mutator(layer2_transform), Mutator(layer1_transform)});

  layer1->Paint(paint_context());
  mock_canvas().ExpectDrawCalls(
      {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
       MockCanvas::DrawCall{1, MockCanvas::ConcatMatrixData{layer1_transform}},
       MockCanvas::DrawCall{
           1, MockCanvas::DrawPathData{child_path, SkPaint(SkColors::kBlue)}},
       MockCanvas::DrawCall{1, MockCanvas::SaveData{2}},
       MockCanvas::DrawCall{2, MockCanvas::ConcatMatrixData{layer2_transform}},
       MockCanvas::DrawCall{
           2, MockCanvas::DrawPathData{child_path, SkPaint(SkColors::kGreen)}},
       MockCanvas::DrawCall{2, MockCanvas::RestoreData{1}},
       MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}});
}

}  // namespace testing
}  // namespace flutter
