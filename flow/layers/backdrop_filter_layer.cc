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

void BackdropFilterLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "BackdropFilterLayer::Paint");
  FML_DCHECK(needs_painting());

  if (retainedBackdrop_ && context.raster_cache && context.surface &&
      context.view_embedder == nullptr) {
    SkCanvas* canvas = context.leaf_nodes_canvas;
    RasterCacheResult my_cache = context.raster_cache->GetSnapshot(
        retainedBackdrop_.get(), context.gr_context, canvas->getTotalMatrix(),
        context.surface, filter_.get());
    if (my_cache.is_valid()) {
      my_cache.drawSnapshot(canvas);
      SkAutoCanvasRestore auto_restore(canvas, true);
      PaintChildren(context);
      return;
    }
  }

  // FML_LOG(ERROR) << "filtering using autosavelayer";
  Layer::AutoSaveLayer save = Layer::AutoSaveLayer::Create(
      context,
      SkCanvas::SaveLayerRec{&paint_bounds(), nullptr, filter_.get(), 0});
  PaintChildren(context);
}

}  // namespace flutter
