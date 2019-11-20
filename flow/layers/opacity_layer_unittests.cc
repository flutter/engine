// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/opacity_layer.h"

#include "flutter/flow/testing/layer_test.h"
#include "flutter/flow/testing/mock_layer.h"
#include "flutter/fml/macros.h"
#include "flutter/testing/mock_canvas.h"

namespace flutter {
namespace testing {

using OpacityLayerDeathTest = LayerTest;
using OpacityLayerTest = LayerTest;

TEST_F(OpacityLayerDeathTest, LeafLayer) {
  auto layer =
      std::make_shared<OpacityLayer>(SK_AlphaOPAQUE, SkPoint::Make(0.0f, 0.0f));

  EXPECT_DEATH_IF_SUPPORTED(layer->Preroll(preroll_context(), SkMatrix()),
                            "layers\\(\\)\\.size\\(\\)\\ > 0");
}

TEST_F(OpacityLayerDeathTest, EmptyLayer) {
  auto mock_layer = std::make_shared<MockLayer>(SkPath());
  auto layer =
      std::make_shared<OpacityLayer>(SK_AlphaOPAQUE, SkPoint::Make(0.0f, 0.0f));
  layer->Add(mock_layer);

  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(mock_layer->paint_bounds(), SkPath().getBounds());
  EXPECT_EQ(layer->paint_bounds(), mock_layer->paint_bounds());
  EXPECT_FALSE(mock_layer->needs_painting());
  EXPECT_FALSE(layer->needs_painting());

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}

TEST_F(OpacityLayerDeathTest, PaintBeforePreroll) {
  SkPath child_path;
  child_path.addRect(5.0f, 6.0f, 20.5f, 21.5f);
  auto mock_layer = std::make_shared<MockLayer>(child_path);
  auto layer =
      std::make_shared<OpacityLayer>(SK_AlphaOPAQUE, SkPoint::Make(0.0f, 0.0f));
  layer->Add(mock_layer);

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(\\)");
}

TEST_F(OpacityLayerTest, FullyOpaque) {
  const SkPoint child_offset = SkPoint::Make(0.5f, 1.5f);
  const SkRect child_bounds = SkRect::MakeLTRB(5.0f, 6.0f, 20.5f, 21.5f);
  const SkPath child_path = SkPath().addRect(child_bounds);
  const SkMatrix child_transform =
      SkMatrix::MakeTrans(child_offset.fX, child_offset.fY);
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  const SkMatrix integral_child_transform =
      RasterCache::GetIntegralTransCTM(child_transform);
#endif
  const SkMatrix initial_transform = SkMatrix::MakeTrans(-0.5f, -0.5f);
  const SkPaint child_paint = SkPaint(SkColors::kGreen);
  auto mock_layer = std::make_shared<MockLayer>(child_path, child_paint);
  auto layer = std::make_shared<OpacityLayer>(SK_AlphaOPAQUE, child_offset);
  layer->Add(mock_layer);

  const SkRect layer_bounds = child_transform.mapRect(child_path.getBounds());
  layer->Preroll(preroll_context(), initial_transform);
  EXPECT_EQ(mock_layer->paint_bounds(), child_path.getBounds());
  EXPECT_EQ(layer->paint_bounds(), layer_bounds);
  EXPECT_TRUE(mock_layer->needs_painting());
  EXPECT_TRUE(layer->needs_painting());
  mock_layer->ExpectParentMatrix(
      SkMatrix::Concat(initial_transform, child_transform));
  mock_layer->ExpectMutators(
      {Mutator(SK_AlphaOPAQUE), Mutator(child_transform)});

  const SkPaint opacity_paint = SkPaint(SkColors::kBlack);  // A = 1.0f
  SkRect opacity_bounds;
  layer_bounds.makeOffset(-child_offset.fX, -child_offset.fY)
      .roundOut(&opacity_bounds);
  layer->Paint(paint_context());
  mock_canvas().ExpectDrawCalls(
      {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
       MockCanvas::DrawCall{1, MockCanvas::ConcatMatrixData{child_transform}},
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
       MockCanvas::DrawCall{
           1, MockCanvas::SetMatrixData{integral_child_transform}},
#endif
       MockCanvas::DrawCall{
           1, MockCanvas::SaveLayerData{opacity_bounds, opacity_paint, nullptr,
                                        2}},
       MockCanvas::DrawCall{2,
                            MockCanvas::DrawPathData{child_path, child_paint}},
       MockCanvas::DrawCall{2, MockCanvas::RestoreData{1}},
       MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}});
}

}  // namespace testing
}  // namespace flutter
