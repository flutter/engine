// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/testing/layer_test.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

using AutoSaveLayerTests = LayerTest;

TEST_F(AutoSaveLayerTests, SaveLayerOnInternalNodesCanvasByDefault) {
  // For:
  //[[nodiscard]] static AutoSaveLayer Create(const PaintContext& paint_context,
  //                                          const SkRect& bounds,
  //                                          const SkPaint* paint,
  //                                          bool save_on_leaf_canvas = false);
  {
    int saved_count_before =
        paint_context().internal_nodes_canvas->getSaveCount();
    {
      const SkPaint paint;
      const SkRect rect = SkRect::MakeEmpty();
      Layer::AutoSaveLayer save =
          Layer::AutoSaveLayer::Create(paint_context(), rect, &paint);
      EXPECT_EQ(paint_context().internal_nodes_canvas->getSaveCount(),
                saved_count_before + 1);
      EXPECT_EQ(paint_context().leaf_nodes_canvas->getSaveCount(),
                saved_count_before + 1);
    }
    EXPECT_EQ(paint_context().internal_nodes_canvas->getSaveCount(),
              saved_count_before);
    EXPECT_EQ(paint_context().leaf_nodes_canvas->getSaveCount(),
              saved_count_before);
  }
  // For:
  // [[nodiscard]] static AutoSaveLayer Create(const PaintContext&
  // paint_context,
  //                                          const SkCanvas::SaveLayerRec&
  //                                          layer_rec, bool
  //                                          save_on_leaf_canvas = false);
  {
    int saved_count_before =
        paint_context().internal_nodes_canvas->getSaveCount();
    {
      const SkPaint paint;
      const SkRect rect = SkRect::MakeEmpty();
      const SkCanvas::SaveLayerRec save_layer_rect =
          SkCanvas::SaveLayerRec{&rect, &paint, nullptr, 0};
      Layer::AutoSaveLayer save =
          Layer::AutoSaveLayer::Create(paint_context(), save_layer_rect);
      EXPECT_EQ(paint_context().internal_nodes_canvas->getSaveCount(),
                saved_count_before + 1);
      EXPECT_EQ(paint_context().leaf_nodes_canvas->getSaveCount(),
                saved_count_before + 1);
    }
    EXPECT_EQ(paint_context().internal_nodes_canvas->getSaveCount(),
              saved_count_before);
    EXPECT_EQ(paint_context().leaf_nodes_canvas->getSaveCount(),
              saved_count_before);
  }
}

TEST_F(AutoSaveLayerTests, SaveLayerOnlyOnLeafNodesCanvas) {
  // For:
  //[[nodiscard]] static AutoSaveLayer Create(const PaintContext& paint_context,
  //                                          const SkRect& bounds,
  //                                          const SkPaint* paint,
  //                                          bool save_on_leaf_canvas = false);
  {
    int saved_count_before =
        paint_context().internal_nodes_canvas->getSaveCount();
    {
      const SkPaint paint;
      const SkRect rect = SkRect::MakeEmpty();
      Layer::AutoSaveLayer save =
          Layer::AutoSaveLayer::Create(paint_context(), rect, &paint, true);
      EXPECT_EQ(paint_context().internal_nodes_canvas->getSaveCount(),
                saved_count_before);
      EXPECT_EQ(paint_context().leaf_nodes_canvas->getSaveCount(),
                saved_count_before + 1);
    }
    EXPECT_EQ(paint_context().internal_nodes_canvas->getSaveCount(),
              saved_count_before);
    EXPECT_EQ(paint_context().leaf_nodes_canvas->getSaveCount(),
              saved_count_before);
  }
  // For:
  // [[nodiscard]] static AutoSaveLayer Create(const PaintContext&
  // paint_context,
  //                                          const SkCanvas::SaveLayerRec&
  //                                          layer_rec, bool
  //                                          save_on_leaf_canvas = false);
  {
    int saved_count_before =
        paint_context().internal_nodes_canvas->getSaveCount();
    {
      const SkPaint paint;
      const SkRect rect = SkRect::MakeEmpty();
      const SkCanvas::SaveLayerRec save_layer_rect =
          SkCanvas::SaveLayerRec{&rect, &paint, nullptr, 0};
      Layer::AutoSaveLayer save =
          Layer::AutoSaveLayer::Create(paint_context(), save_layer_rect, true);
      EXPECT_EQ(paint_context().internal_nodes_canvas->getSaveCount(),
                saved_count_before);
      EXPECT_EQ(paint_context().leaf_nodes_canvas->getSaveCount(),
                saved_count_before + 1);
    }
    EXPECT_EQ(paint_context().internal_nodes_canvas->getSaveCount(),
              saved_count_before);
    EXPECT_EQ(paint_context().leaf_nodes_canvas->getSaveCount(),
              saved_count_before);
  }
}

}  // namespace testing
}  // namespace flutter
