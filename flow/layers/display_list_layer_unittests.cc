// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/flow/layers/display_list_layer.h"

#include "flutter/display_list/display_list_builder.h"
#include "flutter/flow/testing/diff_context_test.h"
#include "flutter/flow/testing/skia_gpu_object_layer_test.h"
#include "flutter/fml/macros.h"
#include "flutter/testing/mock_canvas.h"

#ifndef SUPPORT_FRACTIONAL_TRANSLATION
#include "flutter/flow/raster_cache.h"
#endif

namespace flutter {
namespace testing {

using DisplayListLayerTest = SkiaGPUObjectLayerTest;

#ifndef NDEBUG
TEST_F(DisplayListLayerTest, PaintBeforePrerollInvalidDisplayListDies) {
  const SkPoint layer_offset = SkPoint::Make(0.0f, 0.0f);
  auto layer = std::make_shared<DisplayListLayer>(
      layer_offset, SkiaGPUObject<DisplayList>(), false, false);

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "display_list_\\.skia_object\\(\\)");
}

TEST_F(DisplayListLayerTest, PaintBeforePrerollDies) {
  const SkPoint layer_offset = SkPoint::Make(0.0f, 0.0f);
  const SkRect picture_bounds = SkRect::MakeLTRB(5.0f, 6.0f, 20.5f, 21.5f);
  DisplayListBuilder builder;
  builder.drawRect(picture_bounds);
  auto display_list = builder.Build();
  auto layer = std::make_shared<DisplayListLayer>(
      layer_offset, SkiaGPUObject<DisplayList>(display_list, unref_queue()),
      false, false);

  EXPECT_EQ(layer->paint_bounds(), SkRect::MakeEmpty());
  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(context\\)");
}

TEST_F(DisplayListLayerTest, PaintingEmptyLayerDies) {
  const SkPoint layer_offset = SkPoint::Make(0.0f, 0.0f);
  const SkRect picture_bounds = SkRect::MakeEmpty();
  DisplayListBuilder builder;
  builder.drawRect(picture_bounds);
  auto display_list = builder.Build();
  auto layer = std::make_shared<DisplayListLayer>(
      layer_offset, SkiaGPUObject<DisplayList>(display_list, unref_queue()),
      false, false);

  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(layer->paint_bounds(), SkRect::MakeEmpty());
  EXPECT_FALSE(layer->needs_painting(paint_context()));

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()),
                            "needs_painting\\(context\\)");
}

TEST_F(DisplayListLayerTest, InvalidDisplayListDies) {
  const SkPoint layer_offset = SkPoint::Make(0.0f, 0.0f);
  auto layer = std::make_shared<DisplayListLayer>(
      layer_offset, SkiaGPUObject<DisplayList>(), false, false);

  // Crashes reading a nullptr.
  EXPECT_DEATH_IF_SUPPORTED(layer->Preroll(preroll_context(), SkMatrix()), "");
}
#endif

TEST_F(DisplayListLayerTest, SimpleDisplayList) {
  const SkPoint layer_offset = SkPoint::Make(1.5f, -0.5f);
  const SkMatrix layer_offset_matrix =
      SkMatrix::Translate(layer_offset.fX, layer_offset.fY);
  const SkRect picture_bounds = SkRect::MakeLTRB(5.0f, 6.0f, 20.5f, 21.5f);
  DisplayListBuilder builder;
  builder.drawRect(picture_bounds);
  auto display_list = builder.Build();
  auto layer = std::make_shared<DisplayListLayer>(
      layer_offset, SkiaGPUObject(display_list, unref_queue()), false, false);

  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(layer->paint_bounds(),
            picture_bounds.makeOffset(layer_offset.fX, layer_offset.fY));
  EXPECT_EQ(layer->display_list(), display_list.get());
  EXPECT_TRUE(layer->needs_painting(paint_context()));

  layer->Paint(paint_context());
  auto expected_draw_calls = std::vector(
      {MockCanvas::DrawCall{0, MockCanvas::SaveData{1}},
       MockCanvas::DrawCall{
           1, MockCanvas::ConcatMatrixData{SkM44(layer_offset_matrix)}},
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
       MockCanvas::DrawCall{
           1, MockCanvas::SetMatrixData{SkM44(
                  RasterCache::GetIntegralTransCTM(layer_offset_matrix))}},
#endif
       MockCanvas::DrawCall{
           1, MockCanvas::DrawRectData{picture_bounds, SkPaint()}},
       MockCanvas::DrawCall{1, MockCanvas::RestoreData{0}}});
  EXPECT_EQ(mock_canvas().draw_calls(), expected_draw_calls);
}

using DisplayListLayerDiffTest = DiffContextTest;

TEST_F(DisplayListLayerDiffTest, SimpleDisplayList) {
  auto display_list = CreateDisplayList(SkRect::MakeLTRB(10, 10, 60, 60), 1);

  MockLayerTree tree1;
  tree1.root()->Add(CreateDisplayListLayer(display_list));

  auto damage = DiffLayerTree(tree1, MockLayerTree());
  EXPECT_EQ(damage.frame_damage, SkIRect::MakeLTRB(10, 10, 60, 60));

  MockLayerTree tree2;
  tree2.root()->Add(CreateDisplayListLayer(display_list));

  damage = DiffLayerTree(tree2, tree1);
  EXPECT_TRUE(damage.frame_damage.isEmpty());

  MockLayerTree tree3;
  damage = DiffLayerTree(tree3, tree2);
  EXPECT_EQ(damage.frame_damage, SkIRect::MakeLTRB(10, 10, 60, 60));
}

TEST_F(DisplayListLayerDiffTest, FractionalTranslation) {
  auto display_list = CreateDisplayList(SkRect::MakeLTRB(10, 10, 60, 60), 1);

  MockLayerTree tree1;
  tree1.root()->Add(
      CreateDisplayListLayer(display_list, SkPoint::Make(0.5, 0.5)));

  auto damage = DiffLayerTree(tree1, MockLayerTree());
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  EXPECT_EQ(damage.frame_damage, SkIRect::MakeLTRB(11, 11, 61, 61));
#else
  EXPECT_EQ(damage.frame_damage, SkIRect::MakeLTRB(10, 10, 61, 61));
#endif
}

TEST_F(DisplayListLayerDiffTest, DisplayListCompare) {
  MockLayerTree tree1;
  auto display_list1 = CreateDisplayList(SkRect::MakeLTRB(10, 10, 60, 60), 1);
  tree1.root()->Add(CreateDisplayListLayer(display_list1));

  auto damage = DiffLayerTree(tree1, MockLayerTree());
  EXPECT_EQ(damage.frame_damage, SkIRect::MakeLTRB(10, 10, 60, 60));

  MockLayerTree tree2;
  auto display_list2 = CreateDisplayList(SkRect::MakeLTRB(10, 10, 60, 60), 1);
  tree2.root()->Add(CreateDisplayListLayer(display_list2));

  damage = DiffLayerTree(tree2, tree1);
  EXPECT_EQ(damage.frame_damage, SkIRect::MakeEmpty());

  MockLayerTree tree3;
  auto display_list3 = CreateDisplayList(SkRect::MakeLTRB(10, 10, 60, 60), 1);
  // add offset
  tree3.root()->Add(
      CreateDisplayListLayer(display_list3, SkPoint::Make(10, 10)));

  damage = DiffLayerTree(tree3, tree2);
  EXPECT_EQ(damage.frame_damage, SkIRect::MakeLTRB(10, 10, 70, 70));

  MockLayerTree tree4;
  // different color
  auto display_list4 = CreateDisplayList(SkRect::MakeLTRB(10, 10, 60, 60), 2);
  tree4.root()->Add(
      CreateDisplayListLayer(display_list4, SkPoint::Make(10, 10)));

  damage = DiffLayerTree(tree4, tree3);
  EXPECT_EQ(damage.frame_damage, SkIRect::MakeLTRB(20, 20, 70, 70));
}

TEST_F(DisplayListLayerTest, RenderToWithDefaultOpacity) {
  DisplayListBuilder builder;
  builder.setColor(SK_ColorRED);
  builder.drawRect({10, 10, 50, 50});
  auto display_list = builder.Build();
  ASSERT_TRUE(display_list->can_apply_group_opacity());

  SkPoint layer_offset = SkPoint::Make(10, 10);
  auto dl_layer = DisplayListLayer(
      layer_offset, SkiaGPUObject(display_list, unref_queue()), false, true);

  dl_layer.Preroll(preroll_context(), SkMatrix::I());

  DisplayListCanvasRecorder recorder({0, 0, 100, 100});
  SkNWayCanvas internal_nodes_canvas(100, 100);
  FixedRefreshRateStopwatch raster_time;
  FixedRefreshRateStopwatch ui_time;
  flutter::Layer::PaintContext paint_context = {
      &internal_nodes_canvas,    // internal_nodes_canvas
      &recorder,                 // leaf_nodes_canvas
      nullptr,                   // gr_context
      nullptr,                   // external_view_embedder
      raster_time,               // raster_time
      ui_time,                   // ui_time
      texture_regitry(),         // texture_registry
      nullptr,                   // raster_cache
      false,                     // checkerboard_offscreen_layers
      1.0f,                      // frame_device_pixel_ratio;
      1.0f,                      // inherited_opacity
      recorder.builder().get(),  // leaf_nodes_builder
  };
  dl_layer.Paint(paint_context);
  auto layer_display_list = recorder.Build();

  DisplayListBuilder ref_builder;
  // DisplayListLayer setup
  {
    ref_builder.save();
    ref_builder.translate(layer_offset.fX, layer_offset.fY);
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
    ref_builder.transformReset();
    ref_builder.transformFullPerspective(1, 0, 0, 10,  //
                                         0, 1, 0, 10,  //
                                         0, 0, 1, 0,   //
                                         0, 0, 0, 1    //
    );
#endif
  }
  // DisplayList::RenderTo(Builder) setup
  { ref_builder.save(); }
  // DisplayList contents from above
  {
    ref_builder.setColor(SK_ColorRED);
    ref_builder.drawRect({10, 10, 50, 50});
  }
  // DisplayList::RenderTo take down
  {
    ref_builder.restore();
    ref_builder.setColor(0xFF000000);
  }
  // DisplayListLayer auto restore
  { ref_builder.restore(); }
  auto ref_display_list = ref_builder.Build();

  ASSERT_EQ(layer_display_list->op_count(), ref_display_list->op_count());
  ASSERT_EQ(layer_display_list->bytes(), ref_display_list->bytes());
  ASSERT_TRUE(layer_display_list->Equals(*ref_display_list));
}

TEST_F(DisplayListLayerTest, RenderToWithHalfOpacity) {
  DisplayListBuilder builder;
  builder.setColor(SK_ColorRED);
  builder.drawRect({10, 10, 50, 50});
  auto display_list = builder.Build();
  ASSERT_TRUE(display_list->can_apply_group_opacity());

  SkPoint layer_offset = SkPoint::Make(10, 10);
  auto dl_layer = DisplayListLayer(
      layer_offset, SkiaGPUObject(display_list, unref_queue()), false, true);
  dl_layer.Preroll(preroll_context(), SkMatrix::I());
  DisplayListCanvasRecorder recorder({0, 0, 100, 100});
  SkNWayCanvas internal_nodes_canvas(100, 100);
  FixedRefreshRateStopwatch raster_time;
  FixedRefreshRateStopwatch ui_time;
  flutter::Layer::PaintContext paint_context = {
      &internal_nodes_canvas,    // internal_nodes_canvas
      &recorder,                 // leaf_nodes_canvas
      nullptr,                   // gr_context
      nullptr,                   // external_view_embedder
      raster_time,               // raster_time
      ui_time,                   // ui_time
      texture_regitry(),         // texture_registry
      nullptr,                   // raster_cache
      false,                     // checkerboard_offscreen_layers
      1.0f,                      // frame_device_pixel_ratio;
      0.5f,                      // inherited_opacity
      recorder.builder().get(),  // leaf_nodes_builder
  };
  dl_layer.Paint(paint_context);

  auto layer_display_list = recorder.Build();

  DisplayListBuilder ref_builder;
  // DisplayListLayer setup
  {
    ref_builder.save();
    ref_builder.translate(layer_offset.fX, layer_offset.fY);
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
    ref_builder.transformReset();
    ref_builder.transformFullPerspective(1, 0, 0, 10,  //
                                         0, 1, 0, 10,  //
                                         0, 0, 1, 0,   //
                                         0, 0, 0, 1    //
    );
#endif
  }
  // DisplayList::RenderTo(Builder) setup
  {
    ref_builder.setColor(0x80000000);  // For saveLayer
    ref_builder.saveLayer(&display_list->bounds(), true);
    ref_builder.setColor(0xFF000000);  // Due to reset to defaults
  }
  // DisplayList contents from above
  {
    ref_builder.setColor(SK_ColorRED);
    ref_builder.drawRect({10, 10, 50, 50});
  }
  // DisplayList::RenderTo take down
  {
    ref_builder.restore();
    ref_builder.setColor(0xFF000000);
  }
  // DisplayListLayer auto restore
  { ref_builder.restore(); }
  auto ref_display_list = ref_builder.Build();

  ASSERT_EQ(layer_display_list->op_count(), ref_display_list->op_count());
  ASSERT_EQ(layer_display_list->bytes(), ref_display_list->bytes());
  ASSERT_TRUE(layer_display_list->Equals(*ref_display_list));
}

}  // namespace testing
}  // namespace flutter
