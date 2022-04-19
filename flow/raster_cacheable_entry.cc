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
  auto* layer = cacheable_item_->asLayer();
  auto strategy = cache_children_ ? RasterCacheLayerStrategy::kLayerChildren
                                  : RasterCacheLayerStrategy::kLayer;
  if (auto* cache = context->raster_cache) {
    if (touch_cache_) {
      // Don't evict raster cache entry during partial repaint
      cache->Touch(layer, matrix, strategy);
      return;
    }
    cache->Prepare(context, layer, matrix, strategy);
  }
}

void CacheableDisplayListWrapper::TryToPrepareRasterCache(
    PrerollContext* context,
    const SkMatrix& matrix) {
  if (auto* cache = context->raster_cache) {
    if (touch_cache_) {
      // Don't evict raster cache entry during partial repaint
      cache->Touch(display_list_, matrix);
      return;
    }
    cache->Prepare(context, display_list_, matrix);
  }
}

void CacheableSkPictureWrapper::TryToPrepareRasterCache(
    PrerollContext* context,
    const SkMatrix& matrix) {
  if (auto* cache = context->raster_cache) {
    if (touch_cache_) {
      // Don't evict raster cache entry during partial repaint
      cache->Touch(sk_picture_, matrix);
      return;
    }
    cache->Prepare(context, sk_picture_, matrix);
  }
}

RasterCacheableEntry::RasterCacheableEntry(
    std::unique_ptr<CacheableItemWrapperBase> item,
    const PrerollContext& context,
    const SkMatrix& matrix,
    unsigned num_child,
    bool need_caching)
    : matrix(matrix),
      cull_rect(context.cull_rect),
      num_child_entries(num_child),
      need_caching(need_caching),
      has_platform_view(context.has_platform_view),
      has_texture_layer(context.has_texture_layer),
      item_(std::move(item)) {}

void RasterCacheableEntry::TryToPrepareRasterCache(PrerollContext* context) {
  context->has_platform_view = has_platform_view;
  context->has_texture_layer = has_texture_layer;
  context->cull_rect = cull_rect;

  item_->TryToPrepareRasterCache(context, matrix);
}

}  // namespace flutter
