// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/picture_raster_cache_item.h"

#include <optional>
#include <utility>

#include "flutter/flow/layers/layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/raster_cache.h"
#include "flutter/flow/raster_cache_item.h"
#include "flutter/flow/raster_cache_key.h"

#include "flutter/flow/raster_cache_util.h"
#include "include/core/SkCanvas.h"

namespace flutter {

bool IsPictureWorthRasterizing(SkPicture* picture,
                               bool will_change,
                               bool is_complex) {
  if (will_change) {
    // If the picture is going to change in the future, there is no point in
    // doing to extra work to rasterize.
    return false;
  }

  if (picture == nullptr ||
      !RasterCacheUtil::CanRasterizeRect(picture->cullRect())) {
    // No point in deciding whether the picture is worth rasterizing if it
    // cannot be rasterized at all.
    return false;
  }

  if (is_complex) {
    // The caller seems to have extra information about the picture and thinks
    // the picture is always worth rasterizing.
    return true;
  }

  // TODO(abarth): We should find a better heuristic here that lets us avoid
  // wasting memory on trivial layers that are easy to re-rasterize every frame.
  return picture->approximateOpCount(true) > 5;
}

SkPictureRasterCacheItem::SkPictureRasterCacheItem(SkPicture* sk_picture,
                                                   const SkPoint& offset,
                                                   bool is_complex,
                                                   bool will_change)
    : RasterCacheItem(RasterCacheKeyID(sk_picture->uniqueID(),
                                       RasterCacheKeyType::kPicture),
                      CacheState::kCurrent),
      sk_picture_(sk_picture),
      offset_(offset),
      is_complex_(is_complex),
      will_change_(will_change) {}

std::unique_ptr<SkPictureRasterCacheItem> SkPictureRasterCacheItem::Make(
    SkPicture* sk_picture,
    const SkPoint& offset,
    bool is_complex,
    bool will_change) {
  return std::make_unique<SkPictureRasterCacheItem>(sk_picture, offset,
                                                    is_complex, will_change);
}

void SkPictureRasterCacheItem::PrerollSetup(PrerollContext* context,
                                            const SkMatrix& matrix) {
  cache_state_ = CacheState::kNone;
  if (!IsPictureWorthRasterizing(sk_picture_, will_change_, is_complex_)) {
    // We only deal with pictures that are worthy of rasterization.
    return;
  }
  transformation_matrix_ = matrix;
  transformation_matrix_.preTranslate(offset_.x(), offset_.y());
  if (!transformation_matrix_.invert(nullptr)) {
    // The matrix was singular. No point in going further.
    return;
  }
  if (context->raster_cached_entries && context->raster_cache) {
    context->raster_cached_entries->push_back(this);
    cache_state_ = CacheState::kCurrent;
  }

  return;
}

void SkPictureRasterCacheItem::PrerollFinalize(PrerollContext* context,
                                               const SkMatrix& matrix) {
  if (cache_state_ == CacheState::kNone || !context->raster_cache ||
      !context->raster_cached_entries) {
    return;
  }
  if (!context->raster_cache->GenerateNewCacheInThisFrame()) {
    cache_state_ = CacheState::kNone;
    return;
  }
  auto* raster_cache = context->raster_cache;
  SkRect bounds = sk_picture_->cullRect().makeOffset(offset_.x(), offset_.y());
  // We've marked the cache entry that we would like to cache so it stays
  // alive, but if the following conditions apply then we need to set our
  // state back to !cacheable_ so that we don't populate the entry later.
  if (!SkRect::Intersects(context->cull_rect, bounds)) {
    cache_state_ = CacheState::kNone;
    return;
  }

  // Frame threshold has not yet been reached.
  if (raster_cache->MarkSeen(key_id_, transformation_matrix_) <
      context->raster_cache->access_threshold()) {
    cache_state_ = CacheState::kNone;
  } else {
    context->subtree_can_inherit_opacity = true;
    cache_state_ = CacheState::kCurrent;
  }
  return;
}

bool SkPictureRasterCacheItem::Draw(const PaintContext& context,
                                    const SkPaint* paint) const {
  return Draw(context, context.leaf_nodes_canvas, paint);
}

bool SkPictureRasterCacheItem::Draw(const PaintContext& context,
                                    SkCanvas* canvas,
                                    const SkPaint* paint) const {
  if (!context.raster_cache || !canvas) {
    return false;
  }
  context.raster_cache->Touch(key_id_, canvas->getTotalMatrix());
  if (cache_state_ == CacheState::kCurrent) {
    return context.raster_cache->Draw(key_id_, *canvas, paint);
  }
  return false;
}

static const auto* flow_type = "RasterCacheFlow::Picture";

bool SkPictureRasterCacheItem::TryToPrepareRasterCache(
    const PaintContext& context,
    bool parent_cached) const {
  if (!context.raster_cache || parent_cached) {
    return false;
  }
  if (cache_state_ != kNone &&
      context.raster_cache->MarkSeen(key_id_, transformation_matrix_)) {
    auto transformation_matrix = transformation_matrix_;
// GetIntegralTransCTM effect for matrix which only contains scale,
// translate, so it won't affect result of matrix decomposition and cache
// key.
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
    transformation_matrix =
        RasterCacheUtil::GetIntegralTransCTM(transformation_matrix);
#endif
    SkRect bounds = sk_picture_->cullRect().makeInset(offset_.fX, offset_.fY);
    RasterCache::Context r_context = {
        // clang-format off
      .gr_context         = context.gr_context,
      .dst_color_space    = context.dst_color_space,
      .matrix             = transformation_matrix,
      .logical_rect       = bounds,
      .flow_type          = flow_type,
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
