// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/testing/mock_raster_cache.h"

#include "third_party/skia/include/core/SkPictureRecorder.h"

namespace flutter {
namespace testing {

MockRasterCacheResult::MockRasterCacheResult(SkRect device_rect)
    : RasterCacheResult(nullptr, SkRect::MakeEmpty(), "RasterCacheFlow::test"),
      device_rect_(device_rect) {}

std::unique_ptr<RasterCacheResult> MockRasterCache::RasterizePicture(
    SkPicture* picture,
    GrDirectContext* context,
    const SkMatrix& ctm,
    SkColorSpace* dst_color_space,
    bool checkerboard) const {
  SkRect logical_rect = picture->cullRect();
  SkRect cache_rect = RasterCache::GetDeviceBounds(logical_rect, ctm);

  return std::make_unique<MockRasterCacheResult>(cache_rect);
}

std::unique_ptr<RasterCacheResult> MockRasterCache::RasterizeDisplayList(
    DisplayList* display_list,
    GrDirectContext* context,
    const SkMatrix& ctm,
    SkColorSpace* dst_color_space,
    bool checkerboard) const {
  SkRect logical_rect = display_list->bounds();
  SkRect cache_rect = RasterCache::GetDeviceBounds(logical_rect, ctm);

  return std::make_unique<MockRasterCacheResult>(cache_rect);
}

void MockRasterCache::AddMockLayer(int width, int height) {
  SkMatrix ctm = SkMatrix::I();
  SkPath path;
  path.addRect(100, 100, 100 + width, 100 + height);
  MockCacheableLayer layer = MockCacheableLayer(path);
  layer.Preroll(&preroll_context_, ctm);
  // Prepare(layer.GetCacheableLayer(), &paint_context_);
}

void MockRasterCache::AddMockPicture(int width, int height) {
  FML_DCHECK(access_threshold() > 0);
  // SkMatrix ctm = SkMatrix::I();
  SkPictureRecorder skp_recorder;
  SkRTreeFactory rtree_factory;
  SkPath path;
  path.addRect(100, 100, 100 + width, 100 + height);
  SkCanvas* recorder_canvas = skp_recorder.beginRecording(
      SkRect::MakeLTRB(0, 0, 200 + width, 200 + height), &rtree_factory);
  recorder_canvas->drawPath(path, SkPaint());
  sk_sp<SkPicture> picture = skp_recorder.finishRecordingAsPicture();

  PaintContextHolder holder = GetSamplePaintContextHolder(this);
  holder.paint_context.dst_color_space = color_space_;

  // SkPictureCacheableItem picture_item(picture.get(), picture->cullRect(),
  // ctm,
  //                                     true, false);
  // for (size_t i = 0; i < access_threshold(); i++) {
  //   Prepare(&picture_item, &holder.paint_context);
  //   Draw(&picture_item, mock_canvas_);
  // }
  // Prepare(&picture_item, &holder.paint_context);
}

PrerollContextHolder GetSamplePrerollContextHolder(RasterCache* raster_cache) {
  FixedRefreshRateStopwatch raster_time;
  FixedRefreshRateStopwatch ui_time;
  MutatorsStack mutators_stack;
  TextureRegistry texture_registry;
  sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
  PrerollContextHolder holder = {
      {
          raster_cache,               /* raster_cache */
          nullptr,                    /* gr_context */
          nullptr,                    /* external_view_embedder */
          mutators_stack, srgb.get(), /* color_space */
          kGiantRect,                 /* cull_rect */
          false,                      /* layer reads from surface */
          raster_time, ui_time, texture_registry,
          false, /* checkerboard_offscreen_layers */
          1.0f,  /* frame_device_pixel_ratio */
          false, /* has_platform_view */
      },
      srgb};

  return holder;
}

PaintContextHolder GetSamplePaintContextHolder(RasterCache* raster_cache) {
  FixedRefreshRateStopwatch raster_time;
  FixedRefreshRateStopwatch ui_time;
  MutatorsStack mutators_stack;
  TextureRegistry texture_registry;
  sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
  PaintContextHolder holder = {{
                                   // clang-format off
          .internal_nodes_canvas         = nullptr,
          .leaf_nodes_canvas             = nullptr,
          .gr_context                    = nullptr,
          .dst_color_space               = srgb.get(),
          .view_embedder                 = nullptr,
          .raster_time                   = raster_time,
          .ui_time                       = ui_time,
          .texture_registry              = texture_registry,
          .raster_cache                  = raster_cache,
          .checkerboard_offscreen_layers = false,
          .frame_device_pixel_ratio      = 1.0f,
          .inherited_opacity             = SK_Scalar1,
                                   // clang-format on
                               },
                               srgb};

  return holder;
}

}  // namespace testing
}  // namespace flutter
