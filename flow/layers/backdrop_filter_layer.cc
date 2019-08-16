// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/backdrop_filter_layer.h"

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

  bool need_backdrop_paint = true;
  bool need_snapshot = false;
  if (retainedBackdrop_ && context.surface &&
      context.view_embedder == nullptr) {
    RasterCacheResult my_cache =
        context.raster_cache->Get(retainedBackdrop_.get(), SkMatrix::I());
    if (my_cache.is_valid()) {
      my_cache.drawSnapshot(*context.leaf_nodes_canvas);
      need_backdrop_paint = false;
    } else {
      need_snapshot = true;
    }
  }

  if (need_backdrop_paint) {
    Layer::AutoSaveLayer save = Layer::AutoSaveLayer::Create(
        context,
        SkCanvas::SaveLayerRec{&paint_bounds(), nullptr, filter_.get(), 0});
  }

  if (need_snapshot) {
    SkIRect dev_clip;
    context.leaf_nodes_canvas->getDeviceClipBounds(&dev_clip);
    context.raster_cache->PutSnapshot(retainedBackdrop_.get(), context.surface,
                                      dev_clip);
  }

  SkAutoCanvasRestore save(context.leaf_nodes_canvas, true);
  PaintChildren(context);
}

}  // namespace flutter
