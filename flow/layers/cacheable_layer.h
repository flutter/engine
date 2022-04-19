// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_CACHEABLE_LAYER_H_
#define FLUTTER_FLOW_LAYERS_CACHEABLE_LAYER_H_

#include <memory>
#include "flutter/flow/embedded_views.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"

namespace flutter {

class Cacheable {
 public:
  Cacheable() = default;

  enum class CacheType { kNone, kCurrent, kChildren, kTouch };

  class AutoCache {
   public:
    static AutoCache Create(Cacheable* cacheable,
                            PrerollContext* context,
                            const SkMatrix& matrix);

    static AutoCache Create(DisplayListLayer* display_list,
                            PrerollContext* context,
                            const SkMatrix& matrix,
                            SkPoint offset);

    static AutoCache Create(PictureLayer* display_list,
                            PrerollContext* context,
                            const SkMatrix& matrix,
                            SkPoint offset);

    ~AutoCache() {
      cacheable_entry_->num_child_entries =
          context_->raster_cached_entries.size() - current_index_;
      // Get current layer's cache type
      auto cache_type = layer_->NeedCaching(context_, matrix_);
      // we should modify some parmas of the entry, like need_cache or matrix
      layer_->ConfigCacheType(cacheable_entry_, cache_type);
      cacheable_entry_->has_platform_view = context_->has_platform_view;
      cacheable_entry_->has_texture_layer = context_->has_texture_layer;
    }

   private:
    AutoCache(Cacheable* cacheable,
              RasterCacheableEntry* cacheable_entry,
              PrerollContext* context,
              const SkMatrix& matrix)
        : layer_(cacheable), context_(context), matrix_(matrix) {
      cacheable_entry_ = cacheable_entry;
      current_index_ = context_->raster_cached_entries.size();
    }

    int current_index_;
    RasterCacheableEntry* cacheable_entry_ = nullptr;
    Cacheable* layer_ = nullptr;
    PrerollContext* context_ = nullptr;
    const SkMatrix& matrix_;
  };

  virtual Layer* asLayer() = 0;

  virtual CacheType NeedCaching(PrerollContext* context,
                                const SkMatrix& ctm) = 0;

  // Usually, we have this case to do:
  // 1. CacheType::kNone, which mean we don't need to cache this layer, so we
  // set the entry's need_caching to false
  // 2. CacheType::kChildren, which mean we need to cache the layer's children,
  // so we mark children need cache
  virtual void ConfigCacheType(RasterCacheableEntry* entry, CacheType type) {
    if (type == Cacheable::CacheType::kNone) {
      entry->need_caching = false;
    } else if (type == Cacheable::CacheType::kChildren) {
      // Replace Cacheable child
      entry->MarkLayerChildrenNeedCached();
    } else if (type == CacheType::kTouch) {
      // touch the cache
      entry->MarkTouchCache();
    }
  }

  virtual ~Cacheable() = default;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_CACHEABLE_LAYER_H_
