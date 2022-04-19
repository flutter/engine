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
  auto* entry = context->raster_cached_entries
                    .emplace_back(RasterCacheableEntry::MarkLayerCacheable(
                        cacheable, *context, matrix))
                    .get();
  return AutoCache(cacheable, entry, context, matrix);
}

Cacheable::AutoCache Cacheable::AutoCache::Create(
    DisplayListLayer* display_list_layer,
    PrerollContext* context,
    const SkMatrix& matrix,
    SkPoint offset) {
  auto* entry =
      context->raster_cached_entries
          .emplace_back(RasterCacheableEntry::MarkDisplayListCacheable(
              display_list_layer->display_list(), *context, matrix, offset))
          .get();
  return AutoCache(display_list_layer, entry, context, matrix);
}

Cacheable::AutoCache Cacheable::AutoCache::Create(PictureLayer* picture_layer,
                                                  PrerollContext* context,
                                                  const SkMatrix& matrix,
                                                  SkPoint offset) {
  auto* entry = context->raster_cached_entries
                    .emplace_back(RasterCacheableEntry::MarkSkPictureCacheable(
                        picture_layer->picture(), *context, matrix, offset))
                    .get();
  return AutoCache(picture_layer, entry, context, matrix);
}

}  // namespace flutter
