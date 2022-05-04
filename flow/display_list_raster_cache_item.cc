// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/display_list_raster_cache_item.h"

#include <optional>
#include <utility>

#include "flutter/display_list/display_list.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache.h"
#include "flutter/flow/raster_cache_key.h"
#include "flutter/flow/skia_gpu_object.h"

namespace flutter {

SkMatrix transform_matrix(const SkMatrix& matrix) {
  auto transformation_matrix = matrix;
// GetIntegralTransCTM effect for matrix which only contains scale,
// translate, so it won't affect result of matrix decomposition and cache
// key.
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  transformation_matrix =
      RasterCache::GetIntegralTransCTM(transformation_matrix);
#endif
  return transformation_matrix;
}

DisplayListRasterCacheItem::DisplayListRasterCacheItem(
    DisplayList* display_list,
    const SkPoint& offset,
    bool is_complex,
    bool will_change)
    : RasterCacheItem(RasterCacheKeyID(display_list->unique_id(),
                                       RasterCacheKeyType::kDisplayList),
                      CacheState::kCurrent,
                      kDefaultPictureAndDispLayListCacheLimitPerFrame),
      display_list_(display_list),
      offset_(offset),
      is_complex_(is_complex),
      will_change_(will_change) {}

void DisplayListRasterCacheItem::PrerollSetup(PrerollContext* context,
                                              const SkMatrix& matrix) {
  cache_state_ = CacheState::kNone;
  if (!context->raster_cache->GenerateNewCacheInThisFrame()) {
    return;
  }

  transformation_matrix_ = matrix;
  transformation_matrix_.preTranslate(offset_.x(), offset_.y());

  if (!transformation_matrix_.invert(nullptr)) {
    // The matrix was singular. No point in going further.
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

void DisplayListRasterCacheItem::PrerollFinalize(PrerollContext* context,
                                                 const SkMatrix& matrix) {
  if (cache_state_ == CacheState::kNone || !context->raster_cache ||
      !context->raster_cached_entries) {
    return;
  }
  SkRect bounds = display_list_->bounds().makeOffset(offset_.x(), offset_.y());
  // We've marked the cache entry that we would like to cache so it stays
  // alive, but if the following conditions apply then we need to set our
  // state back to !cacheable_ so that we don't populate the entry later.
  if (!SkRect::Intersects(context->cull_rect, bounds)) {
    cache_state_ = CacheState::kNone;
    return;
  }
  if (context->raster_cache) {
    // Frame threshold has not yet been reached.
    if (num_cache_attempts_ < item_cache_threshold_) {
      // Creates an entry, if not present prior.
      context->raster_cache->MarkSeen(key_id_,
                                      transform_matrix(transformation_matrix_));
    } else {
      context->subtree_can_inherit_opacity = true;
    }
    cache_state_ = CacheState::kCurrent;
    return;
  }
}

bool DisplayListRasterCacheItem::Draw(const PaintContext& context,
                                      const SkPaint* paint) const {
  if (cache_state_ == CacheState::kCurrent && context.raster_cache) {
    num_cache_attempts_++;
    return context.raster_cache->Draw(key_id_, *context.leaf_nodes_canvas,
                                      paint);
  }
  return false;
}

bool DisplayListRasterCacheItem::TryToPrepareRasterCache(
    const PaintContext& context) const {
  if (cache_state_ != kNone) {
    SkRect bounds =
        display_list_->bounds().makeOffset(offset_.x(), offset_.y());
    RasterCache::Context r_context = {
        // clang-format off
      .gr_context         = context.gr_context,
      .dst_color_space    = context.dst_color_space,
      .matrix             = transform_matrix(transformation_matrix_),
      .logical_rect       = bounds,
      .checkerboard       = context.checkerboard_offscreen_layers,
        // clang-format on
    };
    context.raster_cache->UpdateCacheEntry(
        GetId().value(), r_context,
        [=](SkCanvas* canvas) { display_list_->RenderTo(canvas); });
  }
  return false;
}
}  // namespace flutter
