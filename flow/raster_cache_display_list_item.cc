// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/raster_cache_display_list_item.h"

#include "flutter/display_list/display_list_complexity.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

RasterCacheDisplayListItem::RasterCacheDisplayListItem(sk_sp<DisplayList> display_list,
                                      const SkPoint offset,
                                      bool is_complex,
                                      bool will_change,
                                      int picture_cache_threshold)
    : display_list_(display_list),
      offset_(offset),
      is_complex_(is_complex),
      will_change_(will_change),
      picture_cache_threshold_(picture_cache_threshold),
      display_list_key_id_(RasterCacheKeyID(display_list->unique_id(),
                                            RasterCacheKeyType::kDisplayList)) {}

void RasterCacheDisplayListItem::PrerollSetup(PrerollContext* context, const SkMatrix& matrix) {
  cacheable_ = false;
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
    unsigned int complexity_score = complexity_calculator->Compute(display_list_.get());
    if (!complexity_calculator->ShouldBeCached(complexity_score)) {
      return;
    }
  }
  if (context->cacheable_item_list && context->raster_cache) {
    context->cacheable_item_list->push_back(this);
    cacheable_ = true;
  }
}

void RasterCacheDisplayListItem::PrerollFinalize(PrerollContext* context, const SkMatrix& matrix) {
  if (!cacheable_ || !context->raster_cache || !context->cacheable_item_list) {
    return;
  }
  if (context->raster_cache) {
    if (context->raster_cache->MarkSeen(display_list_key_id_, matrix)) {
      context->subtree_can_inherit_opacity = true;
      cacheable_ = true;
    } else if (num_cache_attempts_ > picture_cache_threshold_) {
      cacheable_ = true;
    }
  }
  // We've marked the cache entry that we would like to cache so it stays
  // alive, but if the following conditions apply then we need to set our
  // state back to !cacheable_ so that we don't populate the entry later.
  if (!SkRect::Intersects(context->cull_rect, display_list_->bounds())) {
    cacheable_ = false;
  }
}

bool RasterCacheDisplayListItem::PrepareForFrame(const Layer::PaintContext& p_context,
                                                 RasterCache* cache,
                                                 const RasterCache::Context& r_context,
                                                 bool parent_cached) const {
  if (cacheable_) {
    SkRect bounds = display_list_->bounds().makeOffset(offset_.x(), offset_.y());
    return cache->UpdateCacheEntry(display_list_key_id_,
                                   cache_matrix_,
                                   r_context,
                                   bounds,
                                   "RasterCacheFlow::DisplayList",
                                   [=](SkCanvas* canvas) {
      display_list_->RenderTo(canvas);
    });
  }
  return false;
}

bool RasterCacheDisplayListItem::Draw(const Layer::PaintContext& context,
                                      ContainerLayer::AutoCachePaint& paint) const {
  if (cacheable_) {
    return context.raster_cache->Draw(display_list_key_id_,
                                      *context.leaf_nodes_canvas,
                                      paint.paint());
  }
  return false;
}

}  // namespace flutter
