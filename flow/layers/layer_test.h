// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_LAYER_TEST_H_
#define TESTING_LAYER_TEST_H_

#include "flutter/flow/layers/layer.h"
#include "flutter/testing/mock_canvas.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

class LayerTest : public ::testing::Test {
 public:
  LayerTest(MockCanvas* canvas);
  virtual ~LayerTest() = default;

  PrerollContext* preroll_context() { return &preroll_context_; }
  Layer::PaintContext& paint_context() { return paint_context_; }

 protected:
  Stopwatch stopwatch_;
  MutatorsStack mutators_stack_;
  TextureRegistry texture_registry_;
  PrerollContext preroll_context_;
  Layer::PaintContext paint_context_;
};

}  // namespace testing
}  // namespace flutter

#endif  // TESTING_LAYER_TEST_H_
