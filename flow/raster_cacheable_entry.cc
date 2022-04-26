// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/raster_cacheable_entry.h"

#include <utility>

#include "flutter/display_list/display_list.h"
#include "flutter/flow/layers/cacheable_layer.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/raster_cache.h"
#include "flutter/flow/raster_cache_key.h"
#include "include/core/SkMatrix.h"

namespace flutter {

bool CacheableLayerWrapper::TryToPrepareRasterCache(PrerollContext* context,
                                                    const SkMatrix& matrix) {
  auto* layer = cacheable_item_->asLayer();
  auto strategy = cache_children_ ? RasterCacheLayerStrategy::kLayerChildren
                                  : RasterCacheLayerStrategy::kLayer;
  auto* cache = context->raster_cache;
  if (context->has_platform_view || context->has_texture_layer ||
      !SkRect::Intersects(context->cull_rect, layer->paint_bounds())) {
    cache->Touch(layer, matrix, strategy);
    return false;
  }

  return cache->Prepare(context, layer, matrix, strategy);
}

void CacheableLayerWrapper::TouchRasterCache(PrerollContext* context,
                                             const SkMatrix& matrix) {
  auto* layer = cacheable_item_->asLayer();
  auto strategy = cache_children_ ? RasterCacheLayerStrategy::kLayerChildren
                                  : RasterCacheLayerStrategy::kLayer;
  auto* cache = context->raster_cache;
  cache->Touch(layer, matrix, strategy, true);
}

bool CacheableDisplayListWrapper::TryToPrepareRasterCache(
    PrerollContext* context,
    const SkMatrix& matrix) {
  auto* cache = context->raster_cache;
  if (context->cull_rect.intersect(bounds_)) {
    cache->Prepare(context, display_list_, matrix);
    return true;
  }
  // current bound is not intersect with cull_rect, touch the sk_picture
  cache->Touch(display_list_, matrix);
  return false;
}

void CacheableDisplayListWrapper::TouchRasterCache(PrerollContext* context,
                                                   const SkMatrix& matrix) {
  context->raster_cache->Touch(display_list_, matrix, true);
}

bool CacheableSkPictureWrapper::TryToPrepareRasterCache(
    PrerollContext* context,
    const SkMatrix& matrix) {
  auto* cache = context->raster_cache;
  if (context->cull_rect.intersect(bounds_)) {
    cache->Prepare(context, sk_picture_, matrix);
    return true;
  }
  // current bound is not intersect with cull_rect, touch the sk_picture
  cache->Touch(sk_picture_, matrix, true);
  return false;
}

void CacheableSkPictureWrapper::TouchRasterCache(PrerollContext* context,
                                                 const SkMatrix& matrix) {
  context->raster_cache->Touch(sk_picture_, matrix);
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

std::shared_ptr<RasterCacheableEntry> RasterCacheableEntry::MakeLayerCacheable(
    Cacheable* layer,
    const PrerollContext& context,
    const SkMatrix& matrix,
    unsigned num_child,
    bool need_caching) {
  if (!context.raster_cache) {
    return nullptr;
  }
  return std::make_shared<RasterCacheableEntry>(
      std::make_unique<CacheableLayerWrapper>(layer), context, matrix,
      num_child, need_caching);
}

std::shared_ptr<RasterCacheableEntry>
RasterCacheableEntry::MakeDisplayListCacheable(DisplayList* display_list,
                                               const PrerollContext& context,
                                               const SkMatrix& matrix,
                                               SkRect bounds,
                                               unsigned num_child,
                                               bool need_caching) {
  if (!context.raster_cache) {
    return nullptr;
  }
  return std::make_shared<RasterCacheableEntry>(
      std::make_unique<CacheableDisplayListWrapper>(display_list, bounds),
      context, matrix, num_child, need_caching);
}

std::shared_ptr<RasterCacheableEntry>
RasterCacheableEntry::MakeSkPictureCacheable(SkPicture* picture,
                                             const PrerollContext& context,
                                             const SkMatrix& matrix,
                                             SkRect bounds,
                                             unsigned num_child,
                                             bool need_caching) {
  if (!context.raster_cache) {
    return nullptr;
  }
  return std::make_shared<RasterCacheableEntry>(
      std::make_unique<CacheableSkPictureWrapper>(picture, bounds), context,
      matrix, num_child, need_caching);
}

PrerollContext RasterCacheableEntry::MakeEntryPrerollContext(
    PrerollContext* context) {
  return {
      // clang-format off
          .raster_cache                  = context->raster_cache,
          .gr_context                    = context->gr_context,
          .view_embedder                 = context->view_embedder,
          .mutators_stack                = context->mutators_stack,
          .dst_color_space               = context->dst_color_space,
          .cull_rect                     = cull_rect,
          .surface_needs_readback        = false,
          .raster_time                   = context->raster_time,
          .ui_time                       = context->ui_time,
          .texture_registry              = context->texture_registry,
          .checkerboard_offscreen_layers = context->checkerboard_offscreen_layers,
          .frame_device_pixel_ratio      = context->frame_device_pixel_ratio,
          .has_platform_view             = has_platform_view,
          .has_texture_layer             = has_texture_layer,
      // clang-format on
  };
}

}  // namespace flutter
