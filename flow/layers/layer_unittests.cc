// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_path_layer.h"
#include "flutter/flow/layers/clip_rect_layer.h"
#include "flutter/flow/layers/clip_rrect_layer.h"
#include "flutter/flow/layers/color_filter_layer.h"
#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/layers/opacity_layer.h"
#include "flutter/flow/layers/physical_shape_layer.h"
#include "flutter/flow/layers/shader_mask_layer.h"

#include "gtest/gtest.h"

namespace flutter {

class ReadbackLayer : public ContainerLayer {
 public:
  ReadbackLayer(const bool reads, const bool saves) {
    set_layer_reads_surface(reads);
    set_renders_to_save_layer(saves);
  }
  ~ReadbackLayer() override = default;

  static std::shared_ptr<ReadbackLayer> Make(const bool reads,
                                             const bool saves) {
    return std::make_shared<ReadbackLayer>(reads, saves);
  }

  void set_read(const bool reads) { set_layer_reads_surface(reads); }

  void Paint(PaintContext& context) const override {}
};

void TestLayerFlag(bool reads, bool saves) {
  EXPECT_EQ(ReadbackLayer(reads, saves).tree_reads_surface(), reads);
}

void TestChildFlag(bool child_reads, bool uses_save_layer, bool ret) {
  ReadbackLayer parent = ReadbackLayer(false, uses_save_layer);
  parent.Add(ReadbackLayer::Make(child_reads, false));
  EXPECT_EQ(parent.tree_reads_surface(), ret);
}

TEST(Layer, ReadbackFalse) {
  TestLayerFlag(false, false);
  TestLayerFlag(false, true);
}

TEST(Layer, ReadbackTrue) {
  TestLayerFlag(true, false);
  TestLayerFlag(true, true);
}

TEST(Layer, NoReadbackNoSaveLayer) {
  TestChildFlag(false, false, false);
}

TEST(Layer, NoReadbackButSaveLayer) {
  TestChildFlag(false, true, false);
}

TEST(Layer, ReadbackNoSaveLayer) {
  TestChildFlag(true, false, true);
}

TEST(Layer, ReadbackButSaveLayer) {
  TestChildFlag(true, true, false);
}

TEST(Layer, ChildChangesReadback) {
  ReadbackLayer parent = ReadbackLayer(false, false);
  EXPECT_FALSE(parent.tree_reads_surface());
  parent.Add(ReadbackLayer::Make(false, false));
  EXPECT_FALSE(parent.tree_reads_surface());
  std::shared_ptr<ReadbackLayer> child_reads = ReadbackLayer::Make(true, false);
  parent.Add(child_reads);
  EXPECT_TRUE(parent.tree_reads_surface());
  child_reads->set_read(false);
  EXPECT_FALSE(parent.tree_reads_surface());
  child_reads->set_read(true);
  EXPECT_TRUE(parent.tree_reads_surface());
}

void TestClipRect(Clip clip_behavior, bool ret) {
  ClipRectLayer layer = ClipRectLayer(SkRect::MakeWH(5, 5), clip_behavior);
  layer.Add(ReadbackLayer::Make(true, false));
  EXPECT_EQ(layer.tree_reads_surface(), ret);
}

TEST(Layer, ClipRectSaveLayer) {
  //  TestClipRect(Clip::none, true);     // ClipRectLayer asserts !Clip::none
  TestClipRect(Clip::hardEdge, true);
  TestClipRect(Clip::antiAlias, true);
  TestClipRect(Clip::antiAliasWithSaveLayer, false);
}

void TestClipRRect(Clip clip_behavior, bool ret) {
  SkRRect r_rect = SkRRect::MakeRect(SkRect::MakeWH(5, 5));
  ClipRRectLayer layer = ClipRRectLayer(r_rect, clip_behavior);
  layer.Add(ReadbackLayer::Make(true, false));
  EXPECT_EQ(layer.tree_reads_surface(), ret);
}

TEST(Layer, ClipRRectSaveLayer) {
  //  TestClipRRect(Clip::none, true);     // ClipRRectLayer asserts !Clip::none
  TestClipRRect(Clip::hardEdge, true);
  TestClipRRect(Clip::antiAlias, true);
  TestClipRRect(Clip::antiAliasWithSaveLayer, false);
}

void TestClipPath(Clip clip_behavior, bool ret) {
  SkPath path = SkPath();
  path.moveTo(0, 0);
  path.lineTo(5, 0);
  path.lineTo(0, 5);
  path.close();
  ClipPathLayer layer = ClipPathLayer(path, clip_behavior);
  layer.Add(ReadbackLayer::Make(true, false));
  EXPECT_EQ(layer.tree_reads_surface(), ret);
}

TEST(Layer, ClipPathSaveLayer) {
  //  TestClipPath(Clip::none, true);     // ClipRRectLayer asserts !Clip::none
  TestClipPath(Clip::hardEdge, true);
  TestClipPath(Clip::antiAlias, true);
  TestClipPath(Clip::antiAliasWithSaveLayer, false);
}

TEST(Layer, ColorFilterSaveLayer) {
  sk_sp<SkColorFilter> filter = SkColorFilters::LinearToSRGBGamma();
  ColorFilterLayer layer = ColorFilterLayer(filter);
  layer.Add(ReadbackLayer::Make(true, false));
  EXPECT_FALSE(layer.tree_reads_surface());
}

TEST(Layer, OpacitySaveLayer) {
  OpacityLayer layer = OpacityLayer(10, SkPoint::Make(0, 0));
  layer.Add(ReadbackLayer::Make(true, false));
  EXPECT_FALSE(layer.tree_reads_surface());
}

void TestPhysicalShapeLayer(Clip clip_behavior, bool ret) {
  SkPath path = SkPath();
  path.moveTo(0, 0);
  path.lineTo(5, 0);
  path.lineTo(0, 5);
  path.close();
  PhysicalShapeLayer layer = PhysicalShapeLayer(
      SK_ColorRED, SK_ColorBLUE, 1.0f, 100.0f, 10.0f, path, clip_behavior);
  layer.Add(ReadbackLayer::Make(true, false));
  EXPECT_EQ(layer.tree_reads_surface(), ret);
}

TEST(Layer, PhysicalShapeSaveLayer) {
  TestPhysicalShapeLayer(Clip::none, true);
  TestPhysicalShapeLayer(Clip::hardEdge, true);
  TestPhysicalShapeLayer(Clip::antiAlias, true);
  TestPhysicalShapeLayer(Clip::antiAliasWithSaveLayer, false);
}

TEST(Layer, ShaderMaskSaveLayer) {
  ShaderMaskLayer layer = ShaderMaskLayer(
      SkShaders::Empty(), SkRect::MakeWH(5, 5), SkBlendMode::kSrcOver);
  layer.Add(ReadbackLayer::Make(true, false));
  EXPECT_FALSE(layer.tree_reads_surface());
}

}  // namespace flutter
