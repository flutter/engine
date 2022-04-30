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

void Cacheable::InitialCacheableLayerItem(Layer* layer) {
  cacheable_item_ = std::make_unique<LayerCacheableItem>(layer);
}

void Cacheable::InitialCacheableDisplayListItem(DisplayList* display_list,
                                                SkRect bounds,
                                                bool is_complex,
                                                bool will_change) {
  cacheable_item_ = std::make_unique<DisplayListCacheableItem>(
      display_list, bounds, SkMatrix::I(), is_complex, will_change);
}

void Cacheable::InitialCacheableSkPictureItem(SkPicture* sk_picture,
                                              SkRect bounds,
                                              bool is_complex,
                                              bool will_change) {
  cacheable_item_ = std::make_unique<SkPictureCacheableItem>(
      sk_picture, bounds, SkMatrix::I(), is_complex, will_change);
}

Cacheable::AutoCache::~AutoCache() {
  if (skip_) {
    return;
  }
  cacheable_->cacheable_item_->set_child_entries(
      context_->raster_cached_entries->size() - current_index_);
  cacheable_->TryToCache(context_, matrix_);
  if (auto* cacheable_layer = cacheable_->GetCacheableLayer()) {
    cacheable_layer->SetHasPlatformView(context_->has_platform_view);
    cacheable_layer->SetHasTextureLayer(context_->has_texture_layer);
  }
}

Cacheable::AutoCache Cacheable::AutoCache::Create(Cacheable* cacheable,
                                                  PrerollContext* context,
                                                  const SkMatrix& matrix) {
  bool skip = true;
  if (context->raster_cache) {
    cacheable->cacheable_item_->Reset();
    cacheable->cacheable_item_->set_matrix(matrix);
    cacheable->cacheable_item_->set_cull_rect(context->cull_rect);
    context->raster_cached_entries->emplace_back(
        cacheable->cacheable_item_.get());
    skip = false;
  }
  return AutoCache(cacheable, context, matrix, skip);
}

Cacheable::AutoCache Cacheable::AutoCache::Create(
    DisplayListLayer* display_list_layer,
    PrerollContext* context,
    const SkMatrix& matrix,
    SkRect bounds) {
  bool skip = true;
  if (context->raster_cache && display_list_layer->display_list()) {
    display_list_layer->cacheable_item_->Reset();
    display_list_layer->cacheable_item_->set_matrix(matrix);
    display_list_layer->cacheable_item_->set_cull_rect(context->cull_rect);
    context->raster_cached_entries->emplace_back(
        display_list_layer->cacheable_item_.get());
    skip = false;
  }
  return AutoCache(display_list_layer, context, matrix, skip);
}

Cacheable::AutoCache Cacheable::AutoCache::Create(PictureLayer* picture_layer,
                                                  PrerollContext* context,
                                                  const SkMatrix& matrix,
                                                  SkRect bounds) {
  bool skip = true;
  if (context->raster_cache && picture_layer->picture()) {
    picture_layer->cacheable_item_->Reset();
    picture_layer->cacheable_item_->set_matrix(matrix);
    picture_layer->cacheable_item_->set_cull_rect(context->cull_rect);
    context->raster_cached_entries->emplace_back(
        picture_layer->cacheable_item_.get());
    skip = false;
  }
  return AutoCache(picture_layer, context, matrix, skip);
}

}  // namespace flutter
