// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/physical_shape_layer.h"

#include "gtest/gtest.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkCanvasVirtualEnforcer.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/utils/SkNWayCanvas.h"

namespace flutter {

class MockCanvas : public SkCanvasVirtualEnforcer<SkCanvas> {
 public:
  MockCanvas(int width, int height)
    : SkCanvasVirtualEnforcer<SkCanvas>(width, height) {}

protected:
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
      (void)SkCanvasVirtualEnforcer<SkCanvas>::getSaveLayerStrategy(rec);
      return kNoLayer_SaveLayerStrategy;
    }
    bool onDoSaveBehind(const SkRect*) override { FML_DCHECK(false); return false; }

    // No-op overrides for aborting rasterization earlier than SkNullBlitter.
    void onDrawAnnotation(const SkRect&, const char[], SkData*) override { FML_DCHECK(false); }
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override { FML_DCHECK(false); }
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override { FML_DCHECK(false); }
    void onDrawTextBlob(const SkTextBlob*, SkScalar, SkScalar, const SkPaint&) override { FML_DCHECK(false); }
    void onDrawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4], SkBlendMode,
                     const SkPaint&) override { FML_DCHECK(false); }

    void onDrawPaint(const SkPaint&) override { FML_DCHECK(false); }
    void onDrawBehind(const SkPaint&) override { FML_DCHECK(false); }
    void onDrawPoints(PointMode, size_t, const SkPoint[], const SkPaint&) override { FML_DCHECK(false); }
    void onDrawRect(const SkRect&, const SkPaint&) override { FML_DCHECK(false); }
    void onDrawRegion(const SkRegion&, const SkPaint&) override { FML_DCHECK(false); }
    void onDrawOval(const SkRect&, const SkPaint&) override { FML_DCHECK(false); }
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override { FML_DCHECK(false); }
    void onDrawRRect(const SkRRect&, const SkPaint&) override { FML_DCHECK(false); }
    void onDrawPath(const SkPath&, const SkPaint&) override { FML_DCHECK(false); }
    void onDrawBitmap(const SkBitmap&, SkScalar, SkScalar, const SkPaint*) override { FML_DCHECK(false); }
    void onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*,
                          SrcRectConstraint) override { FML_DCHECK(false); }
    void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) override { FML_DCHECK(false); }
    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                         SrcRectConstraint) override { FML_DCHECK(false); }
    void onDrawImageNine(const SkImage*, const SkIRect&, const SkRect&, const SkPaint*) override { FML_DCHECK(false); }
    void onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect&,
                          const SkPaint*) override { FML_DCHECK(false); }
    void onDrawImageLattice(const SkImage*, const Lattice&, const SkRect&,
                            const SkPaint*) override { FML_DCHECK(false); }
    void onDrawBitmapLattice(const SkBitmap&, const Lattice&, const SkRect&,
                             const SkPaint*) override { FML_DCHECK(false); }
    void onDrawVerticesObject(const SkVertices*, const SkVertices::Bone[], int, SkBlendMode,
                              const SkPaint&) override { FML_DCHECK(false); }
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int, SkBlendMode, const SkRect*, const SkPaint*) override { FML_DCHECK(false); }
    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override { FML_DCHECK(false); }
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override { FML_DCHECK(false); }

    void onDrawEdgeAAQuad(const SkRect&, const SkPoint[4], QuadAAFlags, const SkColor4f&,
                          SkBlendMode) override { FML_DCHECK(false); }
    void onDrawEdgeAAImageSet(const ImageSetEntry[], int, const SkPoint[],
                              const SkMatrix[], const SkPaint*, SrcRectConstraint) override { FML_DCHECK(false); }
};

class PhysicalShapeLayerTestMockCanvas : public MockCanvas {
 public:
  struct PathDrawCall {
    SkPath path;
    SkPaint paint;
  };

  struct ShadowRecDrawCall {
    const SkPath& path;
    const SkDrawShadowRec& rec;
  };

  PhysicalShapeLayerTestMockCanvas(int width, int height) : MockCanvas(width, height) {}
  
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

class LayerTest : public testing::Test {
 public:
  LayerTest()
    : canvas_(16, 16),
      internal_nodes_canvas_(canvas_.imageInfo().width(),
                             canvas_.imageInfo().height()),
      preroll_context_({
        nullptr,                  // raster_cache (don't consult the cache)
        nullptr,                  // gr_context (don't care)
        nullptr,                  // external view embedder
        mutator_stack_,           // mutator stack
        canvas_.imageInfo().colorSpace(), // dst_color_space
        kGiantRect,               // SkRect cull_rect
        stopwatch_,               // frame time (dont care)
        stopwatch_,               // engine time (dont care)
        texture_registry_,        // texture registry (not supported)
        false,                    // checkerboard_offscreen_layers
        1000.0f,                  // physical depth
        1.0f,                     // device pixel ratio
        0.0f,                     // total elevation
      }),
      paint_context_({
        &internal_nodes_canvas_,     // internal_nodes_canvase
        &canvas_,                    // leaf_nodes_canvas
        nullptr,                     // gr_context (don't care)
        nullptr,                     // view_embedder (don't care)
        stopwatch_,                  // raster_time (don't care)
        stopwatch_,                  // ui_time (don't care)
        texture_registry_,           // texture_registry (don't care)
        nullptr,                     // raster_cache (don't consult the cache)
        false,                       // checkerboard_offscreen_layers
        10000.0f,                    // frame_physical_depth
        1.0f,                        // frame_device_pixel_ratio
      }) {}

 protected:
  PhysicalShapeLayerTestMockCanvas canvas_;
  const Stopwatch stopwatch_;
  MutatorsStack mutator_stack_;
  TextureRegistry texture_registry_;
  SkNWayCanvas internal_nodes_canvas_;
  PrerollContext preroll_context_;
  Layer::PaintContext paint_context_;
};

using PhysicalShapeLayerTest = LayerTest;

TEST_F(PhysicalShapeLayerTest, EmptyLayer) {
  auto layer = std::make_shared<PhysicalShapeLayer>(SK_ColorGREEN,
                                                    SK_ColorBLACK,
                                                    0.0f,  // elevation
                                                    SkPath(), Clip::none);

  layer->Preroll(&preroll_context_, SkMatrix());
  EXPECT_EQ(layer->paint_bounds(), SkRect::MakeEmpty());
  EXPECT_FALSE(layer->needs_painting());
  EXPECT_FALSE(layer->needs_system_composite());

  EXPECT_DEATH_IF_SUPPORTED(layer->Paint(paint_context_), "");
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

  layer->Paint(paint_context_);
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

  layer->Preroll(&preroll_context_, SkMatrix());
  child_paint_bounds.join(child1->paint_bounds());
  child_paint_bounds.join(child2->paint_bounds());
  EXPECT_EQ(layer->paint_bounds(), layer_path.getBounds());
  EXPECT_NE(layer->paint_bounds(), SkRect::MakeEmpty());
  EXPECT_NE(layer->paint_bounds(), child_paint_bounds);

  layer->Paint(paint_context_);
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

  preroll_context_.frame_physical_depth = 10.0f; // Clamp depth
  layer->Preroll(&preroll_context_, SkMatrix());
  EXPECT_NE(layer->paint_bounds(), layer_path.getBounds());
  EXPECT_EQ(layer->elevation(), 10.0f);
  EXPECT_EQ(layer->total_elevation(), 10.0f);
  EXPECT_TRUE(layer->needs_painting());
  EXPECT_EQ(layer->needs_system_composite(), PhysicalShapeLayerBase::should_system_composite());

  layer->Paint(paint_context_);
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

  preroll_context_.frame_physical_depth = 6.0f; // Clamp depth
  layers[0]->Preroll(&preroll_context_, SkMatrix());
  EXPECT_EQ(layers[0]->elevation(), 1.0f);
  EXPECT_EQ(layers[1]->elevation(), 2.0f);
  EXPECT_EQ(layers[2]->elevation(), 3.0f);
  EXPECT_EQ(layers[3]->elevation(), 2.0f);
  EXPECT_EQ(layers[0]->total_elevation(), 1.0f);
  EXPECT_EQ(layers[1]->total_elevation(), 3.0f);
  EXPECT_EQ(layers[2]->total_elevation(), 4.0f);
  EXPECT_EQ(layers[3]->total_elevation(), 6.0f);

  layers[0]->Paint(paint_context_);
  EXPECT_EQ(canvas_.path_draw_calls().size(), 4u);
  EXPECT_EQ(canvas_.shadow_draw_calls().size(), 4u);
}

}  // namespace flutter
