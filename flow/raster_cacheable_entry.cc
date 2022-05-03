// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/raster_cacheable_entry.h"

#include <optional>
#include <utility>

#include "flutter/display_list/display_list.h"
#include "flutter/flow/layers/cacheable_layer.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/raster_cache.h"
#include "flutter/flow/raster_cache_key.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"

namespace flutter {

void CacheableItem::Touch(const RasterCache* cache) {
  if (GetId().has_value()) {
    cache->Touch(GetId().value(), matrix_);
  }
}

LayerCacheableItem::LayerCacheableItem(Layer* layer, int layer_cached_threshold)
    : CacheableItem(
          RasterCacheKeyID(layer->unique_id(), RasterCacheKeyType::kLayer),
          CacheState::kCurrent,
          layer_cached_threshold),
      layer_(layer) {}

void LayerCacheableItem::PrerollSetup(PrerollContext* context,
                                      const SkMatrix& matrix) {
  if (context->raster_cache && context->raster_cached_entries) {
    child_entries_ = context->raster_cached_entries->size();
    context->raster_cached_entries->push_back(this);
    matrix_ = matrix;
  }
}

void LayerCacheableItem::PrerollFinalize(PrerollContext* context,
                                         const SkMatrix& matrix) {
  cache_state_ = CacheState::kNone;
  if (!context->raster_cache || !context->raster_cached_entries) {
    return;
  }
  child_entries_ = context->raster_cached_entries->size() - child_entries_;
  if (num_cache_attempts_ > item_cache_threshold_) {
    // the layer can be cached
    cache_state_ = CacheState::kCurrent;
    context->raster_cache->MarkSeen(key_id_, matrix);
  } else {
    // access current layer
    num_cache_attempts_++;
    if (can_cache_children_) {
      if (!layer_children_id_.has_value()) {
        auto& children_layers = layer_->as_container_layer()->layers();
        auto children_count = children_layers.size();
        if (children_count == 0) {
          cache_state_ = CacheState::kNone;
          return;
        }
        std::vector<uint64_t> ids;
        std::transform(children_layers.begin(), children_layers.end(),
                       std::back_inserter(ids), [](auto& layer) -> uint64_t {
                         return layer->unique_id();
                       });
        layer_children_id_.emplace(std::move(ids),
                                   RasterCacheKeyType::kLayerChildren);
      }
      cache_state_ = CacheState::kChildren;
      context->raster_cache->MarkSeen(layer_children_id_.value(), matrix);
    }
  }
  // We've marked the cache entry that we would like to cache so it stays
  // alive, but if the following conditions apply then we need to set our
  // state back to kDoNotCache so that we don't populate the entry later.
  if (context->has_platform_view || context->has_texture_layer ||
      !SkRect::Intersects(context->cull_rect, layer_->paint_bounds())) {
    cache_state_ = CacheState::kNone;
  }
}

std::optional<RasterCacheKeyID> LayerCacheableItem::GetId() const {
  switch (cache_state_) {
    case kCurrent:
      return key_id_;
    case kChildren: {
      return layer_children_id_;
    }
    default:
      return std::nullopt;
  }
}

bool LayerCacheableItem::Rasterize(const PaintContext& paint_context,
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

bool LayerCacheableItem::TryToPrepareRasterCache(
    const PaintContext& context) const {
  if (cache_state_ != kNone) {
    RasterCache::Context r_context = {
        // clang-format off
      .gr_context         = context.gr_context,
      .dst_color_space    = context.dst_color_space,
      .matrix             = matrix_,
      .logical_rect       = layer_->paint_bounds(),
      .checkerboard       = context.checkerboard_offscreen_layers,
        // clang-format on
    };
    if (!GetId().has_value()) {
      return false;
    }
    context.raster_cache->UpdateCacheEntry(
        GetId().value(), r_context,
        [=](SkCanvas* canvas) { Rasterize(context, canvas); });
  }
  return false;
}

bool LayerCacheableItem::Draw(const PaintContext& context,
                              const SkPaint* paint) const {
  switch (cache_state_) {
    case CacheableItem::kNone:
      return false;
    case CacheableItem::kCurrent: {
      return context.raster_cache->Draw(key_id_, *context.leaf_nodes_canvas,
                                        paint);
    }
    case CacheableItem::kChildren: {
      return context.raster_cache->Draw(layer_children_id_.value(),
                                        *context.leaf_nodes_canvas, paint);
    }
  }
}

static constexpr int kDefaultPictureAndDispLayListCacheLimitPerFrame = 3;
DisplayListCacheableItem::DisplayListCacheableItem(DisplayList* display_list,
                                                   const SkPoint& offset,
                                                   bool is_complex,
                                                   bool will_change)
    : CacheableItem(RasterCacheKeyID(display_list->unique_id(),
                                     RasterCacheKeyType::kDisplayList),
                    CacheState::kCurrent,
                    kDefaultPictureAndDispLayListCacheLimitPerFrame),
      display_list_(display_list),
      offset_(offset),
      is_complex_(is_complex),
      will_change_(will_change) {}

void DisplayListCacheableItem::PrerollSetup(PrerollContext* context,
                                            const SkMatrix& matrix) {
  cache_state_ = CacheState::kNone;
  if (matrix.invert(nullptr)) {
    return;
  }
  if (will_change_) {
    return;
  }
  if (!RasterCache::CanRasterizeRect(display_list_->bounds())) {
    return;
  }
  if (!is_complex_) {
    DisplayListComplexityCalculator* complexity_calculator =
        context->gr_context ? DisplayListComplexityCalculator::GetForBackend(
                                  context->gr_context->backend())
                            : DisplayListComplexityCalculator::GetForSoftware();
    unsigned int complexity_score =
        complexity_calculator->Compute(display_list_);
    if (!complexity_calculator->ShouldBeCached(complexity_score)) {
      return;
    }
  }
  if (context->raster_cached_entries && context->raster_cache) {
    context->raster_cached_entries->push_back(this);
    cache_state_ = CacheState::kCurrent;
  }
  return;
}

void DisplayListCacheableItem::PrerollFinalize(PrerollContext* context,
                                               const SkMatrix& matrix) {
  if (cache_state_ == CacheState::kNone || !context->raster_cache ||
      !context->raster_cached_entries) {
    return;
  }
  if (context->raster_cache) {
    if (context->raster_cache->MarkSeen(key_id_, matrix)) {
      context->subtree_can_inherit_opacity = true;
      cache_state_ = CacheState::kCurrent;
    } else if (num_cache_attempts_ > item_cache_threshold_) {
      cache_state_ = CacheState::kCurrent;
    }
  }
  // We've marked the cache entry that we would like to cache so it stays
  // alive, but if the following conditions apply then we need to set our
  // state back to !cacheable_ so that we don't populate the entry later.
  if (!SkRect::Intersects(context->cull_rect, display_list_->bounds())) {
    cache_state_ = CacheState::kNone;
  }
}

bool DisplayListCacheableItem::Draw(const PaintContext& context,
                                    const SkPaint* paint) const {
  if (cache_state_ == CacheState::kCurrent) {
    return context.raster_cache->Draw(key_id_, *context.leaf_nodes_canvas,
                                      paint);
  }
  return false;
}

bool DisplayListCacheableItem::TryToPrepareRasterCache(
    const PaintContext& context) const {
  if (cache_state_ != kNone) {
    auto transformation_matrix = matrix_;
// GetIntegralTransCTM effect for matrix which only contains scale,
// translate, so it won't affect result of matrix decomposition and cache
// key.
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
    transformation_matrix =
        RasterCache::GetIntegralTransCTM(transformation_matrix);
#endif
    RasterCache::Context r_context = {
        // clang-format off
      .gr_context         = context.gr_context,
      .dst_color_space    = context.dst_color_space,
      .matrix             = transformation_matrix,
      .logical_rect       = display_list_->bounds(),
      .checkerboard       = context.checkerboard_offscreen_layers,
        // clang-format on
    };
    context.raster_cache->UpdateCacheEntry(
        GetId().value(), r_context,
        [=](SkCanvas* canvas) { display_list_->RenderTo(canvas); });
  }
  return false;
}

SkPictureCacheableItem::SkPictureCacheableItem(SkPicture* sk_picture,
                                               const SkPoint& offset,
                                               bool is_complex,
                                               bool will_change)
    : CacheableItem(RasterCacheKeyID(sk_picture->uniqueID(),
                                     RasterCacheKeyType::kPicture),
                    CacheState::kCurrent,
                    kDefaultPictureAndDispLayListCacheLimitPerFrame),
      sk_picture_(sk_picture),
      offset_(offset),
      is_complex_(is_complex),
      will_change_(will_change) {}

void SkPictureCacheableItem::PrerollSetup(PrerollContext* context,
                                          const SkMatrix& matrix) {
  cache_state_ = CacheState::kNone;
  if (will_change_) {
    // If the picture is going to change in the future, there is no point in
    // doing to extra work to rasterize.
    return;
  }

  if (sk_picture_ == nullptr ||
      !RasterCache::CanRasterizeRect(sk_picture_->cullRect())) {
    // No point in deciding whether the picture is worth rasterizing if it
    // cannot be rasterized at all.
    return;
  }

  if (is_complex_) {
    // The caller seems to have extra information about the picture and thinks
    // the picture is always worth rasterizing.
    cache_state_ = CacheState::kCurrent;
    return;
  }

  // TODO(abarth): We should find a better heuristic here that lets us avoid
  // wasting memory on trivial layers that are easy to re-rasterize every frame.
  cache_state_ = sk_picture_->approximateOpCount(true) > 5
                     ? CacheState::kCurrent
                     : CacheState::kNone;
  return;
}

void SkPictureCacheableItem::PrerollFinalize(PrerollContext* context,
                                             const SkMatrix& matrix) {
  if (cache_state_ == CacheState::kNone || !context->raster_cache ||
      !context->raster_cached_entries) {
    return;
  }
  if (context->raster_cache) {
    if (context->raster_cache->MarkSeen(key_id_, matrix)) {
      context->subtree_can_inherit_opacity = true;
      cache_state_ = CacheState::kCurrent;
    } else if (num_cache_attempts_ > item_cache_threshold_) {
      cache_state_ = CacheState::kCurrent;
    }
  }
  // We've marked the cache entry that we would like to cache so it stays
  // alive, but if the following conditions apply then we need to set our
  // state back to !cacheable_ so that we don't populate the entry later.
  if (!SkRect::Intersects(context->cull_rect, sk_picture_->cullRect())) {
    cache_state_ = CacheState::kNone;
  }
}

bool SkPictureCacheableItem::Draw(const PaintContext& context,
                                  const SkPaint* paint) const {
  if (cache_state_ == CacheState::kCurrent) {
    return context.raster_cache->Draw(key_id_, *context.leaf_nodes_canvas,
                                      paint);
  }
  return false;
}

bool SkPictureCacheableItem::TryToPrepareRasterCache(
    const PaintContext& context) const {
  if (cache_state_ != kNone) {
    auto transformation_matrix = matrix_;
// GetIntegralTransCTM effect for matrix which only contains scale,
// translate, so it won't affect result of matrix decomposition and cache
// key.
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
    transformation_matrix =
        RasterCache::GetIntegralTransCTM(transformation_matrix);
#endif
    RasterCache::Context r_context = {
        // clang-format off
      .gr_context         = context.gr_context,
      .dst_color_space    = context.dst_color_space,
      .matrix             = transformation_matrix,
      .logical_rect       = sk_picture_->cullRect(),
      .checkerboard       = context.checkerboard_offscreen_layers,
        // clang-format on
    };
    context.raster_cache->UpdateCacheEntry(
        GetId().value(), r_context,
        [=](SkCanvas* canvas) { canvas->drawPicture(sk_picture_); });
  }
  return false;
}

}  // namespace flutter
