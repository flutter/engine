// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/raster_cache_item.h"
#include "flutter/flow/display_list_raster_cache_item.h"
#include "flutter/flow/layer_raster_cache_item.h"
#include "flutter/flow/picture_raster_cache_item.h"
#include "flutter/flow/raster_cache.h"

namespace flutter {

std::unique_ptr<LayerRasterCacheItem> RasterCacheItem::MakeLayerRasterCacheItem(
    Layer* layer,
    int layer_cache_threshold,
    bool can_cache_children) {
  return std::make_unique<LayerRasterCacheItem>(layer, layer_cache_threshold,
                                                can_cache_children);
}

std::unique_ptr<DisplayListRasterCacheItem>
RasterCacheItem::MakeDisplayListRasterCacheItem(DisplayList* display_list,
                                                const SkPoint& offset,
                                                bool is_complex,
                                                bool will_change) {
  return std::make_unique<DisplayListRasterCacheItem>(display_list, offset,
                                                      is_complex, will_change);
}

std::unique_ptr<SkPictureRasterCacheItem>
RasterCacheItem::MakeSkPictureRasterCacheItem(SkPicture* sk_picture,
                                              const SkPoint& offset,
                                              bool is_complex,
                                              bool will_change) {
  return std::make_unique<SkPictureRasterCacheItem>(sk_picture, offset,
                                                    is_complex, will_change);
}

}  // namespace flutter
