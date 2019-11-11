// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/opacity_layer.h"

#include "flutter/flow/layers/layer_test.h"
#include "flutter/testing/mock_canvas.h"

namespace flutter {

class OpacityLayerTestMockCanvas : public testing::MockCanvas {
 public:
  struct PathDrawCall {
    SkPath path;
    SkPaint paint;
  };

  OpacityLayerTestMockCanvas(int width, int height)
      : testing::MockCanvas(width, height) {}

  const std::vector<PathDrawCall> path_draw_calls() const {
    return path_draw_calls_;
  }

 private:
  void onDrawPath(const SkPath& path, const SkPaint& paint) override {
    path_draw_calls_.emplace_back(PathDrawCall({path, paint}));
  }

  std::vector<PathDrawCall> path_draw_calls_;
};

class OpacityLayerTest : public testing::LayerTest {
 public:
  OpacityLayerTest() : LayerTest(&canvas_), canvas_(16, 16) {}

 protected:
  OpacityLayerTestMockCanvas canvas_;
};

TEST_F(OpacityLayerTest, EmptyLayer) {
  auto layer =
      std::make_shared<OpacityLayer>(SK_AlphaOPAQUE, SkPoint::Make(0.0f, 0.0f));

  EXPECT_DEATH_IF_SUPPORTED(layer->Preroll(preroll_context(), SkMatrix()), "");
}

}  // namespace flutter
