// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/compositor_context.h"

#include "flutter/flow/layers/layer_tree.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace flutter {

CompositorContext::CompositorContext() = default;

CompositorContext::~CompositorContext() = default;

void CompositorContext::BeginFrame(ScopedFrame& frame,
                                   bool enable_instrumentation) {
  if (enable_instrumentation) {
    frame_count_.Increment();
    raster_time_.Start();
  }
}

void CompositorContext::EndFrame(ScopedFrame& frame,
                                 bool enable_instrumentation) {
  raster_cache_.SweepAfterFrame();
  if (enable_instrumentation) {
    raster_time_.Stop();
  }
}

std::unique_ptr<CompositorContext::ScopedFrame> CompositorContext::AcquireFrame(
    GrContext* gr_context,
    SkCanvas* canvas,
    ExternalViewEmbedder* view_embedder,
    const SkMatrix& root_surface_transformation,
    bool instrumentation_enabled) {
  return std::make_unique<ScopedFrame>(*this, gr_context, canvas, view_embedder,
                                       root_surface_transformation,
                                       instrumentation_enabled);
}

CompositorContext::ScopedFrame::ScopedFrame(
    CompositorContext& context,
    GrContext* gr_context,
    SkCanvas* canvas,
    ExternalViewEmbedder* view_embedder,
    const SkMatrix& root_surface_transformation,
    bool instrumentation_enabled)
    : context_(context),
      gr_context_(gr_context),
      canvas_(canvas),
      view_embedder_(view_embedder),
      root_surface_transformation_(root_surface_transformation),
      instrumentation_enabled_(instrumentation_enabled) {
  context_.BeginFrame(*this, instrumentation_enabled_);
}

CompositorContext::ScopedFrame::~ScopedFrame() {
  context_.EndFrame(*this, instrumentation_enabled_);
}

RasterStatus CompositorContext::ScopedFrame::Raster(
    flutter::LayerTree& layer_tree,
    bool ignore_raster_cache) {
  std::shared_ptr<PrerollRasterOperations> raster_ops =
      std::make_shared<PrerollRasterOperations>();
  layer_tree.Preroll(*this, raster_ops, ignore_raster_cache);

  RasterCache* raster_cache =
      ignore_raster_cache ? nullptr : &context_.raster_cache();
  if (raster_cache) {
    bool checkerboard_offscreen_layers =
        layer_tree.ShouldCheckerboardOffscreenLayers();
    raster_cache->SetCheckboardCacheImages(
        layer_tree.ShouldCheckerboardRasterCacheImages());
    SkColorSpace* color_space =
        canvas() ? canvas()->imageInfo().colorSpace() : nullptr;
    RasterContext raster_context = {raster_cache,
                                    gr_context_,
                                    color_space,
                                    context_.raster_time(),
                                    context_.ui_time(),
                                    context_.texture_registry(),
                                    checkerboard_offscreen_layers};
    for (const auto raster_op : raster_ops->operations) {
      raster_op(&raster_context);
    }
  }

  // Clearing canvas after preroll reduces one render target switch when preroll
  // paints some raster cache.
  if (canvas()) {
    canvas()->clear(SK_ColorTRANSPARENT);
  }
  layer_tree.Paint(*this, ignore_raster_cache);
  return RasterStatus::kSuccess;
}

void CompositorContext::OnGrContextCreated() {
  texture_registry_.OnGrContextCreated();
  raster_cache_.Clear();
}

void CompositorContext::OnGrContextDestroyed() {
  texture_registry_.OnGrContextDestroyed();
  raster_cache_.Clear();
}

}  // namespace flutter
