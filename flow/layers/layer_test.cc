// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer_test.h"

#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/utils/SkNWayCanvas.h"

namespace flutter {
namespace testing {
namespace {

PrerollContext CreatePrerollContext(MockCanvas* canvas,
                                    Stopwatch& stopwatch,
                                    MutatorsStack& mutators_stack,
                                    TextureRegistry& texture_registry) {
  return {
      nullptr,                           // raster_cache (don't care)
      nullptr,                           // gr_context (don't care)
      nullptr,                           // external view embedder
      mutators_stack,                    // mutator stack
      canvas->imageInfo().colorSpace(),  // dst_color_space
      kGiantRect,                        // SkRect cull_rect
      stopwatch,                         // frame time (dont care)
      stopwatch,                         // engine time (dont care)
      texture_registry,                  // texture registry (dont care)
      false,                             // checkerboard_offscreen_layers
      1000.0f,                           // physical depth
      1.0f,                              // device pixel ratio
      0.0f,                              // total elevation
  };
}

Layer::PaintContext CreatePaintContext(MockCanvas* canvas,
                                       Stopwatch& stopwatch,
                                       TextureRegistry& texture_registry) {
  return {
      canvas->GetInternalCanvas(),  // internal_nodes_canvas
      canvas,                       // leaf_nodes_canvas
      nullptr,                      // gr_context (don't care)
      nullptr,                      // view_embedder (don't care)
      stopwatch,                    // raster_time (don't care)
      stopwatch,                    // ui_time (don't care)
      texture_registry,             // texture_registry (don't care)
      nullptr,                      // raster_cache (don't care)
      false,                        // checkerboard_offscreen_layers
      10000.0f,                     // frame_physical_depth
      1.0f,                         // frame_device_pixel_ratio
  };
}

}  // namespace

LayerTest::LayerTest(MockCanvas* canvas)
    : preroll_context_(CreatePrerollContext(canvas,
                                            stopwatch_,
                                            mutators_stack_,
                                            texture_registry_)),
      paint_context_(
          CreatePaintContext(canvas, stopwatch_, texture_registry_)) {}

}  // namespace testing
}  // namespace flutter
