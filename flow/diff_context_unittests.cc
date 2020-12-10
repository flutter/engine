#include "diff_context.h"
#include "flutter/flow/testing/diff_context_test.h"
#include "flutter/flow/testing/mock_layer.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

TEST_F(DiffContextTest, DieIfOldLayerTreeWasNeverDiffed) {
  auto path1 = SkPath().addRect(SkRect::MakeLTRB(0, 0, 50, 50));
  auto c1 = CreateContainerLayer(std::make_shared<MockLayer>(path1));

  MockLayerTree t1;
  t1.root()->Add(c1);

  MockLayerTree t2;
  t2.root()->Add(c1);

  // t1 is used as old_layer_tree, but it was never used used during diffing as
  // current layer tree
  // i.e.
  // DiffLayerTree(t1, LayerTree())
  // That means it contains layers for which the paint regions are not known
  EXPECT_DEATH_IF_SUPPORTED(DiffLayerTree(t2, t1),
                            "Old layer doesn't have paint region");

  // Diff t1 with empty layer tree to determine paint regions
  DiffLayerTree(t1, MockLayerTree());

  // Now we can diff t2 and t1
  auto damage = DiffLayerTree(t2, t1);
  EXPECT_EQ(damage.surface_damage, SkIRect::MakeEmpty());
}

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

}  // namespace testing
}  // namespace flutter
