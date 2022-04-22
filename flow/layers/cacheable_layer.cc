// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/cacheable_layer.h"
#include "flutter/flow/layers/display_list_layer.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/raster_cacheable_entry.h"
namespace flutter {

Cacheable::AutoCache Cacheable::AutoCache::Create(Cacheable* cacheable,
                                                  PrerollContext* context,
                                                  const SkMatrix& matrix) {
  auto cache_entry =
      RasterCacheableEntry::MakeLayerCacheable(cacheable, *context, matrix);
  if (cache_entry) {
    auto* entry_ptr =
        context->raster_cached_entries.emplace_back(std::move(cache_entry))
            .get();
    return AutoCache(cacheable, entry_ptr, context, matrix);
  }
  return AutoCache(cacheable, nullptr, context, matrix);
}

Cacheable::AutoCache Cacheable::AutoCache::Create(
    DisplayListLayer* display_list_layer,
    PrerollContext* context,
    const SkMatrix& matrix,
    SkRect bounds) {
  auto cache_entry = RasterCacheableEntry::MakeDisplayListCacheable(
      display_list_layer->display_list(), *context, matrix, bounds);
  if (cache_entry) {
    auto* entry_ptr =
        context->raster_cached_entries.emplace_back(std::move(cache_entry))
            .get();
    return AutoCache(display_list_layer, entry_ptr, context, matrix);
  }
  return AutoCache(display_list_layer, nullptr, context, matrix);
}

Cacheable::AutoCache Cacheable::AutoCache::Create(PictureLayer* picture_layer,
                                                  PrerollContext* context,
                                                  const SkMatrix& matrix,
                                                  SkRect bounds) {
  auto cache_entry = RasterCacheableEntry::MakeSkPictureCacheable(
      picture_layer->picture(), *context, matrix, bounds);
  if (cache_entry) {
    auto* entry_ptr =
        context->raster_cached_entries.emplace_back(std::move(cache_entry))
            .get();
    return AutoCache(picture_layer, entry_ptr, context, matrix);
  }
  return AutoCache(picture_layer, nullptr, context, matrix);
}

}  // namespace flutter
