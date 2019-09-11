// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/backdrop_filter_layer.h"
#include "flutter/flow/paint_utils.h"

#include "third_party/skia/include/effects/SkImageFilters.h"

namespace flutter {

BackdropFilterLayer::BackdropFilterLayer(sk_sp<SkImageFilter> filter)
    : filter_(std::move(filter)) {}

BackdropFilterLayer::~BackdropFilterLayer() = default;

void BackdropFilterLayer::initRetained(std::shared_ptr<Layer> retainedLayer) {
  BackdropFilterLayer* bdfLayer = (BackdropFilterLayer*)retainedLayer.get();
  if (bdfLayer->retainedBackdrop_) {
    retainedBackdrop_ = bdfLayer->retainedBackdrop_;
  } else {
    retainedBackdrop_ = std::move(retainedLayer);
  }
}

void BackdropFilterLayer::Preroll(PrerollContext* context,
                                  const SkMatrix& matrix) {
  ContainerLayer::Preroll(context, matrix);

  if (retainedBackdrop_ && context->raster_cache &&
      context->view_embedder == nullptr) {
    context->raster_cache->PrepareForSnapshot(context, retainedBackdrop_.get());
  }
}

void BackdropFilterLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "BackdropFilterLayer::Paint");
  FML_DCHECK(needs_painting());

  if (context.surface && context.raster_cache &&
      context.view_embedder == nullptr && retainedBackdrop_) {
    RasterCacheResult my_cache =
        context.raster_cache->Get(retainedBackdrop_.get(), SkMatrix::I());
    if (!my_cache.is_valid()) {
      // FML_LOG(ERROR) << "Grabbing snapshot for layer: " << unique_id();
      const SkImageInfo image_info = context.surface->imageInfo();
      sk_sp<SkImageFilter> transformed_filter =
          filter_.get()->makeWithLocalMatrix(
              context.leaf_nodes_canvas->getTotalMatrix());

      sk_sp<SkSurface> filter_surface = SkSurface::MakeRenderTarget(
          context.gr_context, SkBudgeted::kYes, image_info);

      SkCanvas* filter_canvas = filter_surface->getCanvas();
      SkPaint filter_paint;
      filter_paint.setImageFilter(transformed_filter);
      context.surface->draw(filter_canvas, 0, 0, &filter_paint);

      sk_sp<SkImage> filter_image = filter_surface->makeImageSnapshot();
      my_cache = context.raster_cache->PutSnapshot(retainedBackdrop_.get(),
                                                   filter_image);
      FML_DCHECK(my_cache.is_valid());
    }

    my_cache.drawSnapshot(context.leaf_nodes_canvas);
    SkAutoCanvasRestore auto_restore(context.leaf_nodes_canvas, true);
    PaintChildren(context);
    return;
  }
  // FML_LOG(ERROR) << "filtering using autosavelayer";
  Layer::AutoSaveLayer save = Layer::AutoSaveLayer::Create(
      context,
      SkCanvas::SaveLayerRec{&paint_bounds(), nullptr, filter_.get(), 0});
  PaintChildren(context);
}

}  // namespace flutter
