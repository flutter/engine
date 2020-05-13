// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLOW_TESTING_LAYER_TEST_H_
#define FLOW_TESTING_LAYER_TEST_H_

#include "flutter/flow/layers/layer.h"

#include <optional>
#include <utility>

#include "flutter/fml/macros.h"
#include "flutter/testing/canvas_test.h"
#include "flutter/testing/mock_canvas.h"
#include "flutter/testing/mock_raster_cache.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/utils/SkNWayCanvas.h"

namespace flutter {
namespace testing {

// This fixture allows generating tests which can |Paint()| and |Preroll()|
// |Layer|'s.
// |LayerTest| is a default implementation based on |::testing::Test|.
//
// By default the preroll and paint contexts will not use a raster cache.
// If a test needs to verify the proper operation of a layer in the presence
// of a raster cache then a number of options can be enabled by using the
// following methods:
// @see |use_null_raster_cache()|
// @see |use_mock_raster_cache()|
// @see |use_skia_raster_cache()|
//
// |BaseT| should be the base test type, such as |::testing::Test| below.
template <typename BaseT>
class LayerTestBase : public CanvasTestBase<BaseT> {
  using TestT = CanvasTestBase<BaseT>;

 public:
  LayerTestBase()
      : preroll_context_({
            raster_cache(), /* raster_cache */
            nullptr,        /* gr_context */
            nullptr,        /* external_view_embedder */
            mutators_stack_, TestT::mock_canvas().imageInfo().colorSpace(),
            kGiantRect, /* cull_rect */
            false,      /* layer reads from surface */
            raster_time_, ui_time_, texture_registry_,
            false,  /* checkerboard_offscreen_layers */
            100.0f, /* frame_physical_depth */
            1.0f,   /* frame_device_pixel_ratio */
            0.0f,   /* total_elevation */
            false,  /* has_platform_view */
        }),
        paint_context_({
            TestT::mock_canvas().internal_canvas(), /* internal_nodes_canvas */
            &TestT::mock_canvas(),                  /* leaf_nodes_canvas */
            nullptr,                                /* gr_context */
            nullptr,                                /* external_view_embedder */
            raster_time_, ui_time_, texture_registry_,
            raster_cache(), /* raster_cache */
            false,          /* checkerboard_offscreen_layers */
            100.0f,         /* frame_physical_depth */
            1.0f,           /* frame_device_pixel_ratio */
        }) {}

  // Use no raster cache in subsequent |Preroll| and |Paint| method calls.
  // This is the default mode of operation.
  void use_null_raster_cache() { set_raster_cache(nullptr); }

  // Use a mock raster cache in subsequent |Preroll| and |Paint| method calls.
  // The mock raster cache behaves like a normal raster cache with respect to
  // when layers and pictures are cached, but it does not incur the overhead
  // of rendering the layers or caching the resulting pixels.
  void use_mock_raster_cache() {
    set_raster_cache(
        std::make_unique<RasterCache>(&MockRasterCacheImageDelegate::instance));
  }

  // Use a normal raster cache in subusequent |Preroll| and |Paint| method
  // calls that uses Skia to render cached layers and pictures in the same
  // way as happens when actually handling a frame on a device.
  void use_skia_raster_cache() {
    set_raster_cache(std::make_unique<RasterCache>());
  }

  TextureRegistry& texture_regitry() { return texture_registry_; }
  RasterCache* raster_cache() { return raster_cache_.get(); }
  PrerollContext* preroll_context() { return &preroll_context_; }
  Layer::PaintContext& paint_context() { return paint_context_; }

 private:
  void set_raster_cache(std::unique_ptr<RasterCache> raster_cache) {
    raster_cache_ = std::move(raster_cache);
    preroll_context_.raster_cache = raster_cache_.get();
    paint_context_.raster_cache = raster_cache_.get();
  }

  Stopwatch raster_time_;
  Stopwatch ui_time_;
  MutatorsStack mutators_stack_;
  TextureRegistry texture_registry_;

  std::unique_ptr<RasterCache> raster_cache_;
  PrerollContext preroll_context_;
  Layer::PaintContext paint_context_;

  FML_DISALLOW_COPY_AND_ASSIGN(LayerTestBase);
};
using LayerTest = LayerTestBase<::testing::Test>;

}  // namespace testing
}  // namespace flutter

#endif  // FLOW_TESTING_LAYER_TEST_H_
