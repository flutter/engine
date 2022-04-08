// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/raster_cacheable_entry.h"

#include <utility>

#include "flutter/display_list/display_list.h"
#include "flutter/flow/layers/cacheable_layer.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache.h"
#include "flutter/flow/raster_cache_key.h"
#include "include/core/SkMatrix.h"

namespace flutter {

void CacheableLayerWrapper::TryToPrepareRasterCache(PrerollContext* context,
                                                    const SkMatrix& matrix) {
  layer_->TryToPrepareRasterCache(context, matrix);
}

void CacheableDisplayListWrapper::TryToPrepareRasterCache(
    PrerollContext* context,
    const SkMatrix& matrix) {
  if (auto* cache = context->raster_cache) {
    SkRect bounds =
        display_list_->bounds().makeOffset(offset_.x(), offset_.y());
    TRACE_EVENT0("flutter", "DisplayListLayer::RasterCache");
    if (context->cull_rect.intersects(bounds)) {
      cache->Prepare(context, display_list_, matrix, offset_);
    } else {
      // Don't evict raster cache entry during partial repaint
      cache->Touch(display_list_, matrix);
    }
  }
}

void CacheableSkPictureWrapper::TryToPrepareRasterCache(
    PrerollContext* context,
    const SkMatrix& matrix) {
  if (auto* cache = context->raster_cache) {
    SkRect bounds =
        sk_picture_->cullRect().makeOffset(offset_.x(), offset_.y());

    TRACE_EVENT0("flutter", "PictureLayer::RasterCache (Preroll)");
    if (context->cull_rect.intersects(bounds)) {
      if (cache->Prepare(context, sk_picture_, matrix, offset_)) {
        context->subtree_can_inherit_opacity = true;
      }
    } else {
      // Don't evict raster cache entry during partial repaint
      cache->Touch(sk_picture_, matrix);
    }
  }
}

RasterCacheableEntry::RasterCacheableEntry(
    std::unique_ptr<CacheableItemWrapperBase> item,
    const PrerollContext& context,
    const SkMatrix& matrix,
    unsigned num_child,
    bool need_caching)
    : matrix(matrix),
      mutators_stack(context.mutators_stack),
      cull_rect(context.cull_rect),
      num_child_entries(num_child),
      need_caching(need_caching),
      item_(std::move(item)) {}

// void RasterCacheableEntry::UpdateRasterCacheableEntry(
//     std::unique_ptr<CacheableEntryBase> entry,
//     const PrerollContext& context,
//     const SkMatrix& matrix) {
//   this->entry_ = std::move(entry);
//   this->matrix = matrix;
//   this->mutators_stack = context.mutators_stack;
//   this->cull_rect = context.cull_rect;
//   this->color_space = context.dst_color_space;
// }

void RasterCacheableEntry::TryToPrepareRasterCache(PrerollContext* context) {
  item_->TryToPrepareRasterCache(context, matrix);
}

}  // namespace flutter
