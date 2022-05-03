// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/cacheable_layer.h"
#include <memory>
#include "flutter/flow/layers/display_list_layer.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/raster_cacheable_entry.h"
#include "include/core/SkMatrix.h"

namespace flutter {

void Cacheable::InitialCacheableLayerItem(Layer* layer,
                                          int layer_cache_threshold) {
  cacheable_item_ =
      std::make_unique<LayerCacheableItem>(layer, layer_cache_threshold);
}

void Cacheable::InitialCacheableDisplayListItem(DisplayList* display_list,
                                                const SkPoint& offset,
                                                bool is_complex,
                                                bool will_change) {
  cacheable_item_ = std::make_unique<DisplayListCacheableItem>(
      display_list, offset, is_complex, will_change);
}

void Cacheable::InitialCacheableSkPictureItem(SkPicture* sk_picture,
                                              const SkPoint& offset,
                                              bool is_complex,
                                              bool will_change) {
  cacheable_item_ = std::make_unique<SkPictureCacheableItem>(
      sk_picture, offset, is_complex, will_change);
}

Cacheable::AutoCache::~AutoCache() {
  cacheable_->cacheable_item_->PrerollFinalize(context_, matrix_);
}

Cacheable::AutoCache Cacheable::AutoCache::Create(Cacheable* cacheable,
                                                  PrerollContext* context,
                                                  const SkMatrix& matrix) {
  cacheable->cacheable_item_->PrerollSetup(context, matrix);
  return AutoCache(cacheable, context, matrix);
}

Cacheable::AutoCache Cacheable::AutoCache::Create(
    DisplayListLayer* display_list_layer,
    PrerollContext* context,
    const SkMatrix& matrix) {
  if (context->raster_cache && display_list_layer->display_list()) {
    display_list_layer->cacheable_item_->PrerollSetup(context, matrix);
  }
  return AutoCache(display_list_layer, context, matrix);
}

Cacheable::AutoCache Cacheable::AutoCache::Create(PictureLayer* picture_layer,
                                                  PrerollContext* context,
                                                  const SkMatrix& matrix) {
  if (context->raster_cache && picture_layer->picture()) {
    picture_layer->cacheable_item_->PrerollSetup(context, matrix);
  }
  return AutoCache(picture_layer, context, matrix);
}

}  // namespace flutter
