// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/raster_cache_layer_item.h"
#include "flutter/flow/layers/container_layer.h"

namespace flutter {

RasterCacheLayerItem::RasterCacheLayerItem(CacheableContainerLayer* layer, int layer_cache_threshold)
    : layer_(layer),
      layer_cache_threshold_(layer_cache_threshold),
      layer_key_id_(RasterCacheKeyID(layer->unique_id(),
                                     RasterCacheKeyType::kLayer)),
      children_cacheable_function_(default_children_cacheable_) {}

void RasterCacheLayerItem::PrerollSetup(PrerollContext* context, const SkMatrix& matrix) {
  if (context->cacheable_item_list && context->raster_cache) {
    num_children_cached_ = context->cacheable_item_list->size();
    context->cacheable_item_list->push_back(this);
  }
}

void RasterCacheLayerItem::PrerollFinalize(PrerollContext* context, const SkMatrix& matrix) {
  cache_state_ = CacheState::kDoNotCache;
  if (!context->raster_cache || !context->cacheable_item_list) {
    return;
  }
  num_children_cached_ = context->cacheable_item_list->size() - num_children_cached_;
  if (context->raster_cache) {
    if (num_cache_attempts_ > layer_cache_threshold_) {
      context->raster_cache->MarkSeen(layer_key_id_, matrix);
      cache_state_ = CacheState::kCacheAsLayer;
    } else {
      num_cache_attempts_++;
      if (layer_->canCacheChildren(context, matrix)) {
        if (!layer_children_id_.has_value()) {
          auto children_layers = layer_->layers();
          std::vector<uint64_t> ids;
          std::transform(children_layers.begin(), children_layers.end(),
                         std::back_inserter(ids), [](auto& layer) -> uint64_t {
                           return layer->unique_id();
                         });
          layer_children_id_.emplace(std::move(ids), RasterCacheKeyType::kLayerChildren);
        }
        context->raster_cache->MarkSeen(layer_children_id_.value(), matrix);
        cache_state_ = CacheState::kCacheChildren;
      }
    }
  }
  // We've marked the cache entry that we would like to cache so it stays
  // alive, but if the following conditions apply then we need to set our
  // state back to kDoNotCache so that we don't populate the entry later.
  if (context->has_platform_view || context->has_texture_layer ||
      !SkRect::Intersects(context->cull_rect, layer_->paint_bounds())) {
    cache_state_ = CacheState::kDoNotCache;
  }
}

const std::optional<RasterCacheKeyID> RasterCacheLayerItem::id_() const {
  switch (cache_state_) {
    case CacheState::kDoNotCache:
      FML_DCHECK(cache_state_ != CacheState::kDoNotCache);
      return {};
    case CacheState::kCacheAsLayer:
      return layer_key_id_;
    case CacheState::kCacheChildren:
      return layer_children_id_;
  }
}

bool RasterCacheLayerItem::PrepareForFrame(const Layer::PaintContext& p_context,
                                           RasterCache* cache,
                                           const RasterCache::Context& r_context,
                                           bool parent_cached) const {
  if (cache_state_ != CacheState::kDoNotCache) {
    return cache->UpdateCacheEntry(id_().value(),
                                   cache_matrix_,
                                   r_context,
                                   layer_->paint_bounds(),
                                   "RasterCacheFlow::Layer",
                                   [=](SkCanvas* canvas) {
      RasterizeLayer(p_context, canvas);
    });
  }
  return false;
}

bool RasterCacheLayerItem::RasterizeLayer(const Layer::PaintContext& context, SkCanvas* canvas) const {
  if (cache_state_ == CacheState::kDoNotCache) {
    return false;
  }
  SkISize canvas_size = canvas->getBaseLayerSize();
  SkNWayCanvas internal_nodes_canvas(canvas_size.width(),
                                     canvas_size.height());
  internal_nodes_canvas.setMatrix(canvas->getTotalMatrix());
  internal_nodes_canvas.addCanvas(canvas);
  SkCanvas* nodes_canvas = static_cast<SkCanvas*>(&internal_nodes_canvas);
  Layer::PaintContext paintContext = {
      .internal_nodes_canvas         = nodes_canvas,
      .leaf_nodes_canvas             = canvas,
      .gr_context                    = context.gr_context,
      .view_embedder                 = nullptr,
      .raster_time                   = context.raster_time,
      .ui_time                       = context.ui_time,
      .texture_registry              = context.texture_registry,
      .raster_cache                  = context.raster_cache,
      .checkerboard_offscreen_layers = context.checkerboard_offscreen_layers,
      .frame_device_pixel_ratio      = context.frame_device_pixel_ratio
  };
  switch (cache_state_) {
    case CacheState::kDoNotCache:
      FML_DCHECK(cache_state_ != CacheState::kDoNotCache);
      return false;
    case CacheState::kCacheAsLayer:
      if (layer_->needs_painting(paintContext)) {
        layer_->Paint(paintContext);
      }
      break;
    case CacheState::kCacheChildren:
      layer_->PaintChildren(paintContext);
      break;
  }
  return true;
}

bool RasterCacheLayerItem::Draw(const Layer::PaintContext& context,
                                ContainerLayer::AutoCachePaint& paint) const {
  switch (cache_state_) {
    case CacheState::kDoNotCache:
      return false;
    case CacheState::kCacheAsLayer:
      layer_->updatePaintForLayer(paint);
      return context.raster_cache->Draw(layer_key_id_,
                                        *context.leaf_nodes_canvas,
                                        paint.paint());
    case CacheState::kCacheChildren:
      layer_->updatePaintForChildren(paint);
      return context.raster_cache->Draw(layer_children_id_.value(),
                                        *context.leaf_nodes_canvas,
                                        paint.paint());
  }
}

}  // namespace flutter
