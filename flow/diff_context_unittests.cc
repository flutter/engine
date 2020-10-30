#include "diff_context.h"
#include "flutter/flow/layers/backdrop_filter_layer.h"
#include "flutter/flow/layers/clip_rect_layer.h"
#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/layers/image_filter_layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/layers/transform_layer.h"
#include "flutter/flow/testing/diff_context_test.h"
#include "flutter/flow/testing/mock_layer.h"
#include "gtest/gtest.h"
#include "third_party/skia/include/effects/SkBlurImageFilter.h"

namespace flutter {
namespace testing {

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

TEST_F(DiffContextTest, SimplePicture) {
  auto picture = CreatePicture(SkRect::MakeXYWH(10, 10, 50, 50), 1);

  LayerTree tree1;
  tree1.root()->Add(CreatePictureLayer(picture));

  auto damage = DiffLayerTree(tree1, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(10, 10, 50, 50));

  LayerTree tree2;
  tree2.root()->Add(CreatePictureLayer(picture));

  damage = DiffLayerTree(tree2, tree1);
  EXPECT_TRUE(damage.surface_damage.isEmpty());

  LayerTree tree3;
  damage = DiffLayerTree(tree3, tree2);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(10, 10, 50, 50));
}

TEST_F(DiffContextTest, PictureCompare) {
  LayerTree tree1;
  auto picture1 = CreatePicture(SkRect::MakeXYWH(10, 10, 50, 50), 1);
  tree1.root()->Add(CreatePictureLayer(picture1));

  auto damage = DiffLayerTree(tree1, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(10, 10, 50, 50));

  LayerTree tree2;
  auto picture2 = CreatePicture(SkRect::MakeXYWH(10, 10, 50, 50), 1);
  tree2.root()->Add(CreatePictureLayer(picture2));

  damage = DiffLayerTree(tree2, tree1);
  EXPECT_TRUE(damage.surface_damage.isEmpty());

  LayerTree tree3;
  auto picture3 = CreatePicture(SkRect::MakeXYWH(10, 10, 50, 50), 1);
  // add offset
  tree3.root()->Add(CreatePictureLayer(picture3, SkPoint::Make(10, 10)));

  damage = DiffLayerTree(tree3, tree2);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(10, 10, 60, 60));

  LayerTree tree4;
  // different color
  auto picture4 = CreatePicture(SkRect::MakeXYWH(10, 10, 50, 50), 2);
  tree4.root()->Add(CreatePictureLayer(picture4, SkPoint::Make(10, 10)));

  damage = DiffLayerTree(tree4, tree3);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(20, 20, 50, 50));
}

// Insert PictureLayer amongst container layers
TEST_F(DiffContextTest, PictureLayerInsertion) {
  auto pic1 = CreatePicture(SkRect::MakeXYWH(0, 0, 50, 50), 1);
  auto pic2 = CreatePicture(SkRect::MakeXYWH(100, 0, 50, 50), 1);
  auto pic3 = CreatePicture(SkRect::MakeXYWH(200, 0, 50, 50), 1);

  LayerTree t1;

  auto t1_c1 = CreateContainerLayer(CreatePictureLayer(pic1));
  t1.root()->Add(t1_c1);

  auto t1_c2 = CreateContainerLayer(CreatePictureLayer(pic2));
  t1.root()->Add(t1_c2);

  auto damage = DiffLayerTree(t1, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 150, 50));

  // Add in the middle

  LayerTree t2;
  auto t2_c1 = CreateContainerLayer(CreatePictureLayer(pic1));
  t2_c1->AssignOldLayer(t1_c1.get());
  t2.root()->Add(t2_c1);

  t2.root()->Add(CreatePictureLayer(pic3));

  auto t2_c2 = CreateContainerLayer(CreatePictureLayer(pic2));
  t2_c2->AssignOldLayer(t1_c2.get());
  t2.root()->Add(t2_c2);

  damage = DiffLayerTree(t2, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(200, 0, 50, 50));

  // Add in the beginning

  t2 = LayerTree();
  t2.root()->Add(CreatePictureLayer(pic3));
  t2.root()->Add(t2_c1);
  t2.root()->Add(t2_c2);
  damage = DiffLayerTree(t2, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(200, 0, 50, 50));

  // Add at the end

  t2 = LayerTree();
  t2.root()->Add(t2_c1);
  t2.root()->Add(t2_c2);
  t2.root()->Add(CreatePictureLayer(pic3));
  damage = DiffLayerTree(t2, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(200, 0, 50, 50));
}

// Insert picture layer amongst other picture layers
TEST_F(DiffContextTest, PictureInsertion) {
  auto pic1 = CreatePicture(SkRect::MakeXYWH(0, 0, 50, 50), 1);
  auto pic2 = CreatePicture(SkRect::MakeXYWH(100, 0, 50, 50), 1);
  auto pic3 = CreatePicture(SkRect::MakeXYWH(200, 0, 50, 50), 1);

  LayerTree t1;
  t1.root()->Add(CreatePictureLayer(pic1));
  t1.root()->Add(CreatePictureLayer(pic2));

  auto damage = DiffLayerTree(t1, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 150, 50));

  LayerTree t2;
  t2.root()->Add(CreatePictureLayer(pic3));
  t2.root()->Add(CreatePictureLayer(pic1));
  t2.root()->Add(CreatePictureLayer(pic2));

  damage = DiffLayerTree(t2, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(200, 0, 50, 50));

  LayerTree t3;
  t3.root()->Add(CreatePictureLayer(pic1));
  t3.root()->Add(CreatePictureLayer(pic3));
  t3.root()->Add(CreatePictureLayer(pic2));

  damage = DiffLayerTree(t3, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(200, 0, 50, 50));

  LayerTree t4;
  t4.root()->Add(CreatePictureLayer(pic1));
  t4.root()->Add(CreatePictureLayer(pic2));
  t4.root()->Add(CreatePictureLayer(pic3));

  damage = DiffLayerTree(t4, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(200, 0, 50, 50));
}

TEST_F(DiffContextTest, LayerDeletion) {
  auto path1 = SkPath().addRect(SkRect::MakeXYWH(0, 0, 50, 50));
  auto path2 = SkPath().addRect(SkRect::MakeXYWH(100, 0, 50, 50));
  auto path3 = SkPath().addRect(SkRect::MakeXYWH(200, 0, 50, 50));

  auto c1 = CreateContainerLayer(std::make_shared<MockLayer>(path1));
  auto c2 = CreateContainerLayer(std::make_shared<MockLayer>(path2));
  auto c3 = CreateContainerLayer(std::make_shared<MockLayer>(path3));

  LayerTree t1;
  t1.root()->Add(c1);
  t1.root()->Add(c2);
  t1.root()->Add(c3);

  auto damage = DiffLayerTree(t1, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 250, 50));

  LayerTree t2;
  t2.root()->Add(c2);
  t2.root()->Add(c3);

  damage = DiffLayerTree(t2, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 50, 50));

  LayerTree t3;
  t3.root()->Add(c1);
  t3.root()->Add(c3);

  damage = DiffLayerTree(t3, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(100, 0, 50, 50));

  LayerTree t4;
  t4.root()->Add(c1);
  t4.root()->Add(c2);

  damage = DiffLayerTree(t4, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(200, 0, 50, 50));

  LayerTree t5;
  t5.root()->Add(c1);

  damage = DiffLayerTree(t5, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(100, 0, 150, 50));

  LayerTree t6;
  t6.root()->Add(c2);

  damage = DiffLayerTree(t6, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 250, 50));

  LayerTree t7;
  t7.root()->Add(c3);

  damage = DiffLayerTree(t7, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 150, 50));
}

TEST_F(DiffContextTest, ReplaceLayer) {
  auto path1 = SkPath().addRect(SkRect::MakeXYWH(0, 0, 50, 50));
  auto path2 = SkPath().addRect(SkRect::MakeXYWH(100, 0, 50, 50));
  auto path3 = SkPath().addRect(SkRect::MakeXYWH(200, 0, 50, 50));

  auto path1a = SkPath().addRect(SkRect::MakeXYWH(0, 100, 50, 50));
  auto path2a = SkPath().addRect(SkRect::MakeXYWH(100, 100, 50, 50));
  auto path3a = SkPath().addRect(SkRect::MakeXYWH(200, 100, 50, 50));

  auto c1 = CreateContainerLayer(std::make_shared<MockLayer>(path1));
  auto c2 = CreateContainerLayer(std::make_shared<MockLayer>(path2));
  auto c3 = CreateContainerLayer(std::make_shared<MockLayer>(path3));

  LayerTree t1;
  t1.root()->Add(c1);
  t1.root()->Add(c2);
  t1.root()->Add(c3);

  auto damage = DiffLayerTree(t1, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 250, 50));

  LayerTree t2;
  t2.root()->Add(c1);
  t2.root()->Add(c2);
  t2.root()->Add(c3);

  damage = DiffLayerTree(t2, t1);
  EXPECT_TRUE(damage.surface_damage.isEmpty());

  LayerTree t3;
  t3.root()->Add(CreateContainerLayer({std::make_shared<MockLayer>(path1a)}));
  t3.root()->Add(c2);
  t3.root()->Add(c3);

  damage = DiffLayerTree(t3, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 50, 150));

  LayerTree t4;
  t4.root()->Add(c1);
  t4.root()->Add(CreateContainerLayer(std::make_shared<MockLayer>(path2a)));
  t4.root()->Add(c3);

  damage = DiffLayerTree(t4, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(100, 0, 50, 150));

  LayerTree t5;
  t5.root()->Add(c1);
  t5.root()->Add(c2);
  t5.root()->Add(CreateContainerLayer(std::make_shared<MockLayer>(path3a)));

  damage = DiffLayerTree(t4, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(100, 0, 50, 150));
}

TEST_F(DiffContextTest, DieIfOldLayerWasNeverDiffed) {
  auto path1 = SkPath().addRect(SkRect::MakeXYWH(0, 0, 50, 50));
  auto c1 = CreateContainerLayer(std::make_shared<MockLayer>(path1));

  LayerTree t1;
  t1.root()->Add(c1);

  LayerTree t2;
  t2.root()->Add(c1);

  // t1 is used as old_layer_tree, but it was never used used during diffing as
  // current layer tree
  // i.e.
  // DiffLayerTree(t1, LayerTree())
  // That means it contains layers for which the paint regions are not known
  EXPECT_DEATH_IF_SUPPORTED(DiffLayerTree(t2, t1),
                            "Old layer doesn't have paint region");

  // Diff t1 with empty layer tree to determine paint regions
  DiffLayerTree(t1, LayerTree());

  // Now we can diff t2 and t1
  auto damage = DiffLayerTree(t2, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeEmpty());
}

TEST_F(DiffContextTest, Transform) {
  auto path1 = SkPath().addRect(SkRect::MakeXYWH(0, 0, 50, 50));
  auto m1 = std::make_shared<MockLayer>(path1);

  auto transform1 =
      std::make_shared<TransformLayer>(SkMatrix::MakeTrans(10, 10));
  transform1->Add(m1);

  LayerTree t1;
  t1.root()->Add(transform1);

  auto damage = DiffLayerTree(t1, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(10, 10, 50, 50));

  auto transform2 =
      std::make_shared<TransformLayer>(SkMatrix::MakeTrans(20, 20));
  transform2->Add(m1);
  transform2->AssignOldLayer(transform1.get());

  LayerTree t2;
  t2.root()->Add(transform2);

  damage = DiffLayerTree(t2, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(10, 10, 60, 60));

  auto transform3 =
      std::make_shared<TransformLayer>(SkMatrix::MakeTrans(20, 20));
  transform3->Add(m1);
  transform3->AssignOldLayer(transform2.get());

  LayerTree t3;
  t3.root()->Add(transform3);

  damage = DiffLayerTree(t3, t2);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeEmpty());
}

TEST_F(DiffContextTest, TransformNested) {
  auto path1 = SkPath().addRect(SkRect::MakeXYWH(0, 0, 50, 50));
  auto m1 = CreateContainerLayer(std::make_shared<MockLayer>(path1));

  auto transform1 =
      std::make_shared<TransformLayer>(SkMatrix::MakeScale(2.0, 2.0));

  auto transform1_1 =
      std::make_shared<TransformLayer>(SkMatrix::MakeTrans(10, 10));
  transform1_1->Add(m1);
  transform1->Add(transform1_1);

  auto transform1_2 =
      std::make_shared<TransformLayer>(SkMatrix::MakeTrans(100, 100));
  transform1_2->Add(m1);
  transform1->Add(transform1_2);

  auto transform1_3 =
      std::make_shared<TransformLayer>(SkMatrix::MakeTrans(200, 200));
  transform1_3->Add(m1);
  transform1->Add(transform1_3);

  LayerTree l1;
  l1.root()->Add(transform1);

  auto damage = DiffLayerTree(l1, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(20, 20, 480, 480));

  auto transform2 =
      std::make_shared<TransformLayer>(SkMatrix::MakeScale(2.0, 2.0));

  auto transform2_1 =
      std::make_shared<TransformLayer>(SkMatrix::MakeTrans(10, 10));
  transform2_1->Add(m1);
  transform2_1->AssignOldLayer(transform1_1.get());
  transform2->Add(transform2_1);

  auto transform2_2 =
      std::make_shared<TransformLayer>(SkMatrix::MakeTrans(100, 101));
  transform2_2->Add(m1);
  transform2_2->AssignOldLayer(transform1_2.get());
  transform2->Add(transform2_2);

  auto transform2_3 =
      std::make_shared<TransformLayer>(SkMatrix::MakeTrans(200, 200));
  transform2_3->Add(m1);
  transform2_3->AssignOldLayer(transform1_3.get());
  transform2->Add(transform2_3);

  LayerTree l2;
  l2.root()->Add(transform2);

  damage = DiffLayerTree(l2, l1);

  // transform2 has not transform1 assigned as old layer, so it should be
  // invalidated completely
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(20, 20, 480, 480));

  // now diff the tree properly, the only difference being transform2_2 and
  // transform_2_1
  transform2->AssignOldLayer(transform1.get());
  damage = DiffLayerTree(l2, l1);

  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(200, 200, 100, 102));
}

TEST_F(DiffContextTest, BackdropLayer) {
  auto filter = SkBlurImageFilter::Make(10, 10, nullptr, nullptr,
                                        SkBlurImageFilter::kClamp_TileMode);

  {
    // tests later assume 30px readback area, fail early if that's not the case
    auto readback = filter->filterBounds(SkIRect::MakeWH(10, 10), SkMatrix::I(),
                                         SkImageFilter::kReverse_MapDirection);
    EXPECT_EQ(readback, SkIRect::MakeLTRB(-30, -30, 40, 40));
  }

  LayerTree l1(SkISize::Make(100, 100));
  l1.root()->Add(std::make_shared<BackdropFilterLayer>(filter));

  // no clip, effect over entire surface
  auto damage = DiffLayerTree(l1, LayerTree(SkISize::Make(100, 100)));
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeWH(100, 100));

  LayerTree l2(SkISize::Make(100, 100));

  auto clip = std::make_shared<ClipRectLayer>(SkRect::MakeXYWH(20, 20, 40, 40),
                                              Clip::hardEdge);
  clip->Add(std::make_shared<BackdropFilterLayer>(filter));
  l2.root()->Add(clip);
  damage = DiffLayerTree(l2, LayerTree(SkISize::Make(100, 100)));

  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 90, 90));

  LayerTree l3;
  auto scale = std::make_shared<TransformLayer>(SkMatrix::MakeScale(2.0, 2.0));
  scale->Add(clip);
  l3.root()->Add(scale);

  damage = DiffLayerTree(l3, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(10, 10, 140, 140));

  LayerTree l4;
  l4.root()->Add(scale);

  // path just outside of readback region, doesn't affect blur
  auto path1 = SkPath().addRect(SkRect::MakeXYWH(150, 150, 10, 10));
  l4.root()->Add(std::make_shared<MockLayer>(path1));
  damage = DiffLayerTree(l4, l3);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(150, 150, 10, 10));

  LayerTree l5;
  l5.root()->Add(scale);

  // path just inside of readback region, must trigger backdrop repaint
  auto path2 = SkPath().addRect(SkRect::MakeXYWH(149, 149, 10, 10));
  l5.root()->Add(std::make_shared<MockLayer>(path2));
  damage = DiffLayerTree(l5, l4);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(10, 10, 150, 150));
}

TEST_F(DiffContextTest, ImageFilterLayer) {
  auto filter = SkBlurImageFilter::Make(10, 10, nullptr, nullptr,
                                        SkBlurImageFilter::kClamp_TileMode);

  {
    // tests later assume 30px paint area, fail early if that's not the case
    auto paint_rect =
        filter->filterBounds(SkIRect::MakeWH(10, 10), SkMatrix::I(),
                             SkImageFilter::kForward_MapDirection);
    EXPECT_EQ(paint_rect, SkIRect::MakeLTRB(-30, -30, 40, 40));
  }

  LayerTree l1;
  auto filter_layer = std::make_shared<ImageFilterLayer>(filter);
  auto path = SkPath().addRect(SkRect::MakeXYWH(100, 100, 10, 10));
  filter_layer->Add(std::make_shared<MockLayer>(path));
  l1.root()->Add(filter_layer);

  auto damage = DiffLayerTree(l1, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(70, 70, 70, 70));

  // filter bounds are calculated in screen space, make sure scaling doesn't
  // affect it
  LayerTree l2;
  auto scale = std::make_shared<TransformLayer>(SkMatrix::MakeScale(2.0, 2.0));
  scale->Add(filter_layer);
  l2.root()->Add(scale);

  damage = DiffLayerTree(l2, LayerTree());
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(170, 170, 80, 80));

  LayerTree l3;
  l3.root()->Add(scale);

  // path outside of ImageFilterLayer
  auto path1 = SkPath().addRect(SkRect::MakeXYWH(160, 160, 10, 10));
  l3.root()->Add(std::make_shared<MockLayer>(path1));
  damage = DiffLayerTree(l3, l2);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(160, 160, 10, 10));

  // path intersecting ImageFilterLayer, should trigger ImageFilterLayer repaint
  LayerTree l4;
  l4.root()->Add(scale);
  auto path2 = SkPath().addRect(SkRect::MakeXYWH(160, 160, 11, 11));
  l4.root()->Add(std::make_shared<MockLayer>(path2));
  damage = DiffLayerTree(l4, l3);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(160, 160, 90, 90));
}

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

}  // namespace testing
}  // namespace flutter
