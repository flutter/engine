// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layer_raster_cache_item.h"
#include "flutter/flow/layers/container_layer.h"

namespace flutter {

LayerRasterCacheItem::LayerRasterCacheItem(Layer* layer,
                                           int layer_cached_threshold,
                                           bool can_cache_children)
    : RasterCacheItem(
          RasterCacheKeyID(layer->unique_id(), RasterCacheKeyType::kLayer),
          CacheState::kCurrent),
      layer_(layer),
      layer_cached_threshold_(layer_cached_threshold),
      can_cache_children_(can_cache_children) {}

void LayerRasterCacheItem::PrerollSetup(PrerollContext* context,
                                        const SkMatrix& matrix) {
  if (context->raster_cache && context->raster_cached_entries) {
    context->raster_cached_entries->push_back(this);
    child_entries_ = context->raster_cached_entries->size();
    matrix_ = matrix;
  }
}

void LayerRasterCacheItem::PrerollFinalize(PrerollContext* context,
                                           const SkMatrix& matrix) {
  cache_state_ = CacheState::kNone;
  if (!context->raster_cache || !context->raster_cached_entries) {
    return;
  }
  // We've marked the cache entry that we would like to cache so it stays
  // alive, but if the following conditions apply then we need to set our
  // state back to kDoNotCache so that we don't populate the entry later.
  if (context->has_platform_view || context->has_texture_layer ||
      !SkRect::Intersects(context->cull_rect, layer_->paint_bounds())) {
    return;
  }
  child_entries_ = context->raster_cached_entries->size() - child_entries_;
  if (num_cache_attempts_ >= layer_cached_threshold_) {
    // the layer can be cached
    cache_state_ = CacheState::kCurrent;
    context->raster_cache->MarkSeen(key_id_, matrix);
  } else {
    num_cache_attempts_++;
    // access current layer
    if (can_cache_children_) {
      if (!layer_children_id_.has_value()) {
        auto ids = RasterCacheKeyID::LayerChildrenIds(layer_);
        if (!ids.has_value()) {
          cache_state_ = CacheState::kNone;
          return;
        }
        layer_children_id_.emplace(std::move(ids.value()),
                                   RasterCacheKeyType::kLayerChildren);
      }
      cache_state_ = CacheState::kChildren;
      context->raster_cache->MarkSeen(layer_children_id_.value(), matrix);
    }
  }
}

std::optional<RasterCacheKeyID> LayerRasterCacheItem::GetId() const {
  switch (cache_state_) {
    case kCurrent:
      return key_id_;
    case kChildren: {
      return layer_children_id_;
    }
    default:
      return {};
  }
}

bool LayerRasterCacheItem::Rasterize(const PaintContext& paint_context,
                                     SkCanvas* canvas) const {
  FML_DCHECK(cache_state_ != CacheState::kNone);
  SkISize canvas_size = canvas->getBaseLayerSize();
  SkNWayCanvas internal_nodes_canvas(canvas_size.width(), canvas_size.height());
  internal_nodes_canvas.setMatrix(canvas->getTotalMatrix());
  internal_nodes_canvas.addCanvas(canvas);
  PaintContext context = {
      // clang-format off
          .internal_nodes_canvas         = static_cast<SkCanvas*>(&internal_nodes_canvas),
          .leaf_nodes_canvas             = canvas,
          .gr_context                    = paint_context.gr_context,
          .dst_color_space               = paint_context.dst_color_space,
          .view_embedder                 = paint_context.view_embedder,
          .raster_time                   = paint_context.raster_time,
          .ui_time                       = paint_context.ui_time,
          .texture_registry              = paint_context.texture_registry,
          .raster_cache                  = paint_context.raster_cache,
          .checkerboard_offscreen_layers = paint_context.checkerboard_offscreen_layers,
          .frame_device_pixel_ratio      = paint_context.frame_device_pixel_ratio,
      // clang-format on
  };

  switch (cache_state_) {
    case CacheState::kCurrent:
      if (layer_->needs_painting(context)) {
        layer_->Paint(context);
      }
      break;
    case CacheState::kChildren:
      FML_DCHECK(layer_->as_container_layer());
      layer_->as_container_layer()->PaintChildren(context);
      break;
    case CacheState::kNone:
      FML_DCHECK(cache_state_ != CacheState::kNone);
      return false;
  }
  return true;
}

static const auto* flow_type = "RasterCacheFlow::Layer";

bool LayerRasterCacheItem::TryToPrepareRasterCache(const PaintContext& context,
                                                   bool parent_cached) const {
  if (!context.raster_cache || parent_cached) {
    return false;
  }
  if (cache_state_ != kNone) {
    RasterCache::Context r_context = {
        // clang-format off
      .gr_context         = context.gr_context,
      .dst_color_space    = context.dst_color_space,
      .matrix             = matrix_,
      .logical_rect       = layer_->paint_bounds(),
      .flow_type          = flow_type,
      .checkerboard       = context.checkerboard_offscreen_layers,
        // clang-format on
    };
    return context.raster_cache->UpdateCacheEntry(
        GetId().value(), r_context,
        [=](SkCanvas* canvas) { Rasterize(context, canvas); });
  }
  return false;
}

bool LayerRasterCacheItem::Draw(const PaintContext& context,
                                const SkPaint* paint) const {
  return Draw(context, context.leaf_nodes_canvas, paint);
}

bool LayerRasterCacheItem::Draw(const PaintContext& context,
                                SkCanvas* canvas,
                                const SkPaint* paint) const {
  if (!context.raster_cache || !canvas) {
    return false;
  }
  switch (cache_state_) {
    case RasterCacheItem::kNone:
      return false;
    case RasterCacheItem::kCurrent: {
      return context.raster_cache->Draw(key_id_, *canvas, paint);
    }
    case RasterCacheItem::kChildren: {
      return context.raster_cache->Draw(layer_children_id_.value(), *canvas,
                                        paint);
    }
  }
}

}  // namespace flutter
