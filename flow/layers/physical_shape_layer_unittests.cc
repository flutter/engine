// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/physical_shape_layer.h"

#include "flutter/flow/layers/layer_test.h"
#include "flutter/testing/mock_canvas.h"

namespace flutter {

class PhysicalShapeLayerTestMockCanvas : public testing::MockCanvas {
 public:
  struct PathDrawCall {
    SkPath path;
    SkPaint paint;
  };

  struct ShadowRecDrawCall {
    const SkPath& path;
    const SkDrawShadowRec& rec;
  };

  PhysicalShapeLayerTestMockCanvas(int width, int height) : testing::MockCanvas(width, height) {}

  const std::vector<PathDrawCall> path_draw_calls() const {
    return path_draw_calls_;
  }
  const std::vector<ShadowRecDrawCall> shadow_draw_calls() const {
    return shadow_draw_calls_;
  }

 private:
  void onDrawPath(const SkPath& path, const SkPaint& paint) override {
    path_draw_calls_.emplace_back(PathDrawCall({path, paint}));
  }
  void onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) override {
    shadow_draw_calls_.emplace_back(ShadowRecDrawCall({path, rec}));
  }

  std::vector<PathDrawCall> path_draw_calls_;
  std::vector<ShadowRecDrawCall> shadow_draw_calls_;
};

class PhysicalShapeLayerTest : public testing::LayerTest {
 public:
  PhysicalShapeLayerTest() : LayerTest(&canvas_), canvas_(16, 16) {}

 protected:
  PhysicalShapeLayerTestMockCanvas canvas_;
};

TEST_F(PhysicalShapeLayerTest, EmptyLayer) {
  auto layer = std::make_shared<PhysicalShapeLayer>(SK_ColorGREEN,
                                                    SK_ColorBLACK,
                                                    0.0f,  // elevation
                                                    SkPath(), Clip::none);

  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(layer->paint_bounds(), SkRect::MakeEmpty());
  EXPECT_FALSE(layer->needs_painting());
  EXPECT_FALSE(layer->needs_system_composite());

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context()), "");
}

TEST_F(PhysicalShapeLayerTest, NonEmptyLayer) {
  SkPath layer_path;

  layer_path.addRect(0, 8, 8, 0).close();
  auto layer = std::make_shared<PhysicalShapeLayer>(SK_ColorGREEN, SK_ColorBLACK,
                                                    0.0f,  // elevation
                                                    layer_path,
                                                    Clip::none);
  layer->Preroll(&preroll_context_, SkMatrix());
  EXPECT_EQ(layer->paint_bounds(), layer_path.getBounds());
  EXPECT_TRUE(layer->needs_painting());
  EXPECT_FALSE(layer->needs_system_composite());

  layer->Paint(paint_context());
  EXPECT_EQ(canvas_.path_draw_calls().size(), 1u);
  EXPECT_EQ(canvas_.shadow_draw_calls().size(), 0u);
  EXPECT_EQ(canvas_.path_draw_calls()[0].path, layer_path);
  EXPECT_EQ(canvas_.path_draw_calls()[0].paint.getColor(), SK_ColorGREEN);
}

TEST_F(PhysicalShapeLayerTest, ChildrenLargerThanPath) {
  SkPath layer_path, child1_path, child2_path;
  SkRect child_paint_bounds;

  layer_path.addRect(0, 8, 8, 0).close();
  child1_path.addRect(4, 12, 12, 0).close();
  child2_path.addRect(3, 15, 5, 2).close();
  auto child1 = std::make_shared<PhysicalShapeLayer>(SK_ColorRED,
                                                     SK_ColorBLACK,
                                                     0.0f,  // elevation
                                                     child1_path,
                                                     Clip::none);
  auto child2 = std::make_shared<PhysicalShapeLayer>(SK_ColorBLUE,
                                                     SK_ColorBLACK,
                                                     0.0f,  // elevation
                                                     child2_path,
                                                     Clip::none);
  auto layer = std::make_shared<PhysicalShapeLayer>(SK_ColorGREEN,
                                                    SK_ColorBLACK,
                                                    0.0f,  // elevation
                                                    layer_path,
                                                    Clip::none);
  layer->Add(child1);
  layer->Add(child2);

  layer->Preroll(preroll_context(), SkMatrix());
  child_paint_bounds.join(child1->paint_bounds());
  child_paint_bounds.join(child2->paint_bounds());
  EXPECT_EQ(layer->paint_bounds(), layer_path.getBounds());
  EXPECT_NE(layer->paint_bounds(), SkRect::MakeEmpty());
  EXPECT_NE(layer->paint_bounds(), child_paint_bounds);

  layer->Paint(paint_context());
  EXPECT_EQ(canvas_.path_draw_calls().size(), 3u);
  EXPECT_EQ(canvas_.shadow_draw_calls().size(), 0u);
  EXPECT_EQ(canvas_.path_draw_calls()[0].path, layer_path);
  EXPECT_EQ(canvas_.path_draw_calls()[0].paint.getColor(), SK_ColorGREEN);
  EXPECT_EQ(canvas_.path_draw_calls()[1].path, child1_path);
  EXPECT_EQ(canvas_.path_draw_calls()[1].paint.getColor(), SK_ColorRED);
  EXPECT_EQ(canvas_.path_draw_calls()[2].path, child2_path);
  EXPECT_EQ(canvas_.path_draw_calls()[2].paint.getColor(), SK_ColorBLUE);
}

TEST_F(PhysicalShapeLayerTest, ElevationSimple) {
  SkPath layer_path;

  layer_path.addRect(0, 8, 8, 0).close();
  auto layer = std::make_shared<PhysicalShapeLayer>(SK_ColorGREEN, SK_ColorBLACK,
                                                    20.0f,  // elevation
                                                    layer_path,
                                                    Clip::none);

  preroll_context()->frame_physical_depth = 10.0f; // Clamp depth
  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_NE(layer->paint_bounds(), layer_path.getBounds());
  EXPECT_EQ(layer->elevation(), 10.0f);
  EXPECT_EQ(layer->total_elevation(), 10.0f);
  EXPECT_TRUE(layer->needs_painting());
  EXPECT_EQ(layer->needs_system_composite(), PhysicalShapeLayerBase::should_system_composite());

  layer->Paint(paint_context());
  EXPECT_EQ(canvas_.path_draw_calls().size(), 1u);
  EXPECT_EQ(canvas_.shadow_draw_calls().size(), 1u);
  EXPECT_EQ(canvas_.path_draw_calls()[0].path, layer_path);
  EXPECT_EQ(canvas_.path_draw_calls()[0].paint.getColor(), SK_ColorGREEN);
  EXPECT_EQ(canvas_.shadow_draw_calls()[0].path, layer_path);
}

TEST_F(PhysicalShapeLayerTest, ElevationComplex) {
  // The layer tree should look like this:
  // layers[0] +1.0f = 1.0f
  // |       \
  // |        \
  // |         \
  // |       layers[2] +3.0f = 4.0f
  // |          |
  // |       layers[3] +4.0f = 8.0f (clamped to 6.0f)
  // |
  // |
  // layers[1] + 2.0f = 3.0f
  std::shared_ptr<PhysicalShapeLayer> layers[4];
  for (int i = 0; i < 4; i += 1) {
    layers[i] =
        std::make_shared<PhysicalShapeLayer>(SK_ColorBLACK, SK_ColorBLACK,
                                             (float)(i + 1),  // elevation
                                             SkPath(), Clip::none);
  }
  layers[0]->Add(layers[1]);
  layers[0]->Add(layers[2]);
  layers[2]->Add(layers[3]);

  preroll_context()->frame_physical_depth = 6.0f; // Clamp depth
  layers[0]->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(layers[0]->elevation(), 1.0f);
  EXPECT_EQ(layers[1]->elevation(), 2.0f);
  EXPECT_EQ(layers[2]->elevation(), 3.0f);
  EXPECT_EQ(layers[3]->elevation(), 2.0f);
  EXPECT_EQ(layers[0]->total_elevation(), 1.0f);
  EXPECT_EQ(layers[1]->total_elevation(), 3.0f);
  EXPECT_EQ(layers[2]->total_elevation(), 4.0f);
  EXPECT_EQ(layers[3]->total_elevation(), 6.0f);

  layers[0]->Paint(paint_context());
  EXPECT_EQ(canvas_.path_draw_calls().size(), 4u);
  EXPECT_EQ(canvas_.shadow_draw_calls().size(), 4u);
}

}  // namespace flutter
