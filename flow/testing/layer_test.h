// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLOW_TESTING_LAYER_TEST_H_
#define FLOW_TESTING_LAYER_TEST_H_

#include <optional>
#include <utility>

#include "flutter/flow/layers/layer.h"
#include "flutter/flow/skia_gpu_object.h"
#include "flutter/testing/mock_canvas.h"
#include "flutter/testing/thread_test.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/utils/SkNWayCanvas.h"

namespace flutter {
namespace testing {

template <typename BaseT>
class LayerTestBase : public BaseT {
 public:
  LayerTestBase();
  virtual ~LayerTestBase() = default;

  TextureRegistry& texture_regitry() { return texture_registry_; }
  MockCanvas& mock_canvas() { return canvas_; }
  PrerollContext* preroll_context() { return &preroll_context_; }
  Layer::PaintContext& paint_context() { return paint_context_; }

 private:
  Stopwatch stopwatch_;
  MutatorsStack mutators_stack_;
  TextureRegistry texture_registry_;
  MockCanvas canvas_;

  PrerollContext preroll_context_;
  Layer::PaintContext paint_context_;
};

template <typename BaseT>
LayerTestBase<BaseT>::LayerTestBase()
    : preroll_context_({
          nullptr,                           // raster_cache (don't care)
          nullptr,                           // gr_context (don't care)
          nullptr,                           // external view embedder
          mutators_stack_,                   // mutator stack
          canvas_.imageInfo().colorSpace(),  // dst_color_space
          kGiantRect,                        // SkRect cull_rect
          stopwatch_,                        // frame time (dont care)
          stopwatch_,                        // engine time (dont care)
          texture_registry_,                 // texture registry (dont care)
          false,                             // checkerboard_offscreen_layers
          0.0f,                              // total elevation
      }),
      paint_context_({
          canvas_.internal_canvas(),  // internal_nodes_canvas
          &canvas_,                   // leaf_nodes_canvas
          nullptr,                    // gr_context (don't care)
          nullptr,                    // view_embedder (don't care)
          stopwatch_,                 // raster_time (don't care)
          stopwatch_,                 // ui_time (don't care)
          texture_registry_,          // texture_registry (don't care)
          nullptr,                    // raster_cache (don't care)
          false,                      // checkerboard_offscreen_layers
      }) {}

using LayerTest = LayerTestBase<::testing::Test>;
class SkiaGPUObjectLayerTest : public LayerTestBase<ThreadTest> {
 public:
  SkiaGPUObjectLayerTest()
      : unref_queue_(fml::MakeRefCounted<SkiaUnrefQueue>(
            GetCurrentTaskRunner(),
            fml::TimeDelta::FromSeconds(0))) {}
  ~SkiaGPUObjectLayerTest() override = default;

  fml::RefPtr<SkiaUnrefQueue> unref_queue() { return unref_queue_; }

 private:
  fml::RefPtr<SkiaUnrefQueue> unref_queue_;
};

}  // namespace testing
}  // namespace flutter

#endif  // FLOW_TESTING_LAYER_TEST_H_
