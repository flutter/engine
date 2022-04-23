// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/color_filter_layer.h"
#include "flutter/flow/raster_cacheable_entry.h"

namespace flutter {

ColorFilterLayer::ColorFilterLayer(sk_sp<SkColorFilter> filter)
    : filter_(std::move(filter)), render_count_(1) {}

void ColorFilterLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const ColorFilterLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (filter_ != prev->filter_) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }

#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  context->SetTransform(
      RasterCache::GetIntegralTransCTM(context->GetTransform()));
#endif

  DiffChildren(context, prev);

  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void ColorFilterLayer::Preroll(PrerollContext* context,
                               const SkMatrix& matrix) {
  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context);
  Cacheable::AutoCache cache =
      Cacheable::AutoCache::Create(this, context, matrix);

  ContainerLayer::Preroll(context, matrix);
  // We always use a saveLayer (or a cached rendering), so we
  // can always apply opacity in those cases.
  context->subtree_can_inherit_opacity = true;
}

void ColorFilterLayer::TryToCache(PrerollContext* context,
                                  RasterCacheableEntry* entry,
                                  const SkMatrix& ctm) {
  // We should try to cache the layer or layer's children.
  // First we should to check if we need to touch the cache.
  if (render_count_ >= kMinimumRendersBeforeCachingFilterLayer) {
    // entry default cache current layer
    return;
  }
  render_count_++;
  entry->MarkLayerChildrenNeedCached();
}

void ColorFilterLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "ColorFilterLayer::Paint");
  FML_DCHECK(needs_painting(context));

  AutoCachePaint cache_paint(context);

  if (context.raster_cache) {
    if (context.raster_cache->Draw(this, *context.leaf_nodes_canvas,
                                   RasterCacheLayerStrategy::kLayer,
                                   cache_paint.paint())) {
      return;
    }

    cache_paint.setColorFilter(filter_);
    if (context.raster_cache->Draw(this, *context.leaf_nodes_canvas,
                                   RasterCacheLayerStrategy::kLayerChildren,
                                   cache_paint.paint())) {
      return;
    }
  }
  cache_paint.setColorFilter(filter_);

  Layer::AutoSaveLayer save = Layer::AutoSaveLayer::Create(
      context, paint_bounds(), cache_paint.paint());

  PaintChildren(context);
}

}  // namespace flutter
