// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/picture_raster_cache_item.h"

#include <optional>
#include <utility>

#include "flutter/flow/layers/layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/raster_cache.h"
#include "flutter/flow/raster_cache_key.h"

namespace flutter {

static SkMatrix transform_matrix(const SkMatrix& matrix) {
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

SkPictureRasterCacheItem::SkPictureRasterCacheItem(SkPicture* sk_picture,
                                                   const SkPoint& offset,
                                                   bool is_complex,
                                                   bool will_change)
    : RasterCacheItem(RasterCacheKeyID(sk_picture->uniqueID(),
                                       RasterCacheKeyType::kPicture),
                      CacheState::kCurrent,
                      kDefaultPictureAndDispLayListCacheLimitPerFrame),
      sk_picture_(sk_picture),
      offset_(offset),
      is_complex_(is_complex),
      will_change_(will_change) {}

void SkPictureRasterCacheItem::PrerollSetup(PrerollContext* context,
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

void SkPictureRasterCacheItem::PrerollFinalize(PrerollContext* context,
                                               const SkMatrix& matrix) {
  if (cache_state_ == CacheState::kNone || !context->raster_cache ||
      !context->raster_cached_entries) {
    return;
  }
  SkRect bounds = sk_picture_->cullRect().makeOffset(offset_.x(), offset_.y());
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

bool SkPictureRasterCacheItem::Draw(const PaintContext& context,
                                    const SkPaint* paint) const {
  if (cache_state_ == CacheState::kCurrent && context.raster_cache) {
    num_cache_attempts_++;
    return context.raster_cache->Draw(key_id_, *context.leaf_nodes_canvas,
                                      paint);
  }
  return false;
}

bool SkPictureRasterCacheItem::TryToPrepareRasterCache(
    const PaintContext& context) const {
  if (cache_state_ != kNone) {
    SkRect bounds = sk_picture_->cullRect().makeInset(offset_.fX, offset_.fY);
    RasterCache::Context r_context = {
        // clang-format off
      .gr_context         = context.gr_context,
      .dst_color_space    = context.dst_color_space,
      .matrix             = transform_matrix(transformation_matrix_),
      .logical_rect       = bounds,
      .checkerboard       = context.checkerboard_offscreen_layers,
        // clang-format on
    };
    return context.raster_cache->UpdateCacheEntry(
        GetId().value(), r_context,
        [=](SkCanvas* canvas) { canvas->drawPicture(sk_picture_); });
  }
  return false;
}

}  // namespace flutter
