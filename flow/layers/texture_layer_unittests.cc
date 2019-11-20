// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/texture_layer.h"

#include "flutter/flow/testing/layer_test.h"
#include "flutter/flow/testing/mock_layer.h"
#include "flutter/fml/macros.h"
#include "flutter/testing/mock_canvas.h"

namespace flutter {
namespace testing {

using TextureLayerTest = LayerTest;

TEST_F(TextureLayerTest, InvalidTexture) {
  const SkPoint layer_offset = SkPoint::Make(0.0f, 0.0f);
  const SkSize layer_size = SkSize::Make(8.0f, 8.0f);
  auto layer =
      std::make_shared<TextureLayer>(layer_offset, layer_size, 0, false);

  layer->Preroll(preroll_context(), SkMatrix());
  EXPECT_EQ(layer->paint_bounds(),
            SkRect::MakeSize(layer_size)
                .makeOffset(layer_offset.fX, layer_offset.fY));
  EXPECT_TRUE(layer->needs_painting());

  layer->Paint(paint_context());
  mock_canvas().ExpectDrawCalls({});
}

}  // namespace testing
}  // namespace flutter
