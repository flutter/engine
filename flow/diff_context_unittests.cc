#include "diff_context.h"
#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/testing/diff_context_test.h"
#include "gtest/gtest.h"

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
  auto pic1 = CreatePicture(SkRect::MakeXYWH(0, 0, 50, 50), 1);
  auto pic2 = CreatePicture(SkRect::MakeXYWH(100, 0, 50, 50), 1);
  auto pic3 = CreatePicture(SkRect::MakeXYWH(200, 0, 50, 50), 1);

  auto c1 = CreateContainerLayer(CreatePictureLayer(pic1));
  auto c2 = CreateContainerLayer(CreatePictureLayer(pic2));
  auto c3 = CreateContainerLayer(CreatePictureLayer(pic3));

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
  auto pic1 = CreatePicture(SkRect::MakeXYWH(0, 0, 50, 50), 1);
  auto pic2 = CreatePicture(SkRect::MakeXYWH(100, 0, 50, 50), 1);
  auto pic3 = CreatePicture(SkRect::MakeXYWH(200, 0, 50, 50), 1);

  auto pic1a = CreatePicture(SkRect::MakeXYWH(0, 100, 50, 50), 1);
  auto pic2a = CreatePicture(SkRect::MakeXYWH(100, 100, 50, 50), 1);
  auto pic3a = CreatePicture(SkRect::MakeXYWH(200, 100, 50, 50), 1);

  auto c1 = CreateContainerLayer(CreatePictureLayer(pic1));
  auto c2 = CreateContainerLayer(CreatePictureLayer(pic2));
  auto c3 = CreateContainerLayer(CreatePictureLayer(pic3));

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
  t3.root()->Add(CreateContainerLayer({CreatePictureLayer(pic1a)}));
  t3.root()->Add(c2);
  t3.root()->Add(c3);

  damage = DiffLayerTree(t3, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(0, 0, 50, 150));

  LayerTree t4;
  t4.root()->Add(c1);
  t4.root()->Add(CreateContainerLayer(CreatePictureLayer(pic2a)));
  t4.root()->Add(c3);

  damage = DiffLayerTree(t4, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(100, 0, 50, 150));

  LayerTree t5;
  t5.root()->Add(c1);
  t5.root()->Add(c2);
  t5.root()->Add(CreateContainerLayer(CreatePictureLayer(pic3a)));

  damage = DiffLayerTree(t4, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeXYWH(100, 0, 50, 150));
}

TEST_F(DiffContextTest, DieIfOldLayerWasNeverDiffed) {
  auto pic1 = CreatePicture(SkRect::MakeXYWH(0, 0, 50, 50), 1);
  auto c1 = CreateContainerLayer(CreatePictureLayer(pic1));

  LayerTree t1;
  t1.root()->Add(c1);

  LayerTree t2;
  t2.root()->Add(c1);

  // t1 is used as old_layer_tree, but it was never used used during diffing as
  // current layer tree
  // i.e.
  // DiffLayerTree(t1, LayerTree())
  // That means it contains layers for which the paint regions are not known
  EXPECT_DEATH_IF_SUPPORTED(DiffLayerTree(t2, t1), " region.is_valid\\(\\)");

  // Diff t1 with empty layer tree to determine paint regions
  DiffLayerTree(t1, LayerTree());

  // Now we can diff t2 and t1
  auto damage = DiffLayerTree(t2, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeEmpty());
}

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

}  // namespace testing
}  // namespace flutter
