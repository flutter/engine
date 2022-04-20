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
      layer_->TryToCache(context_, cacheable_entry_, matrix_);
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

  virtual void TryToCache(PrerollContext* context,
                          RasterCacheableEntry* entry,
                          const SkMatrix& ctm) = 0;

  void ShouldTouchCache(PrerollContext* context, RasterCacheableEntry* entry) {
    // check if should touch the cache
    if (context->has_platform_view || context->has_texture_layer ||
        !SkRect::Intersects(context->cull_rect, asLayer()->paint_bounds())) {
      entry->MarkTouchCache();
    }
  }

  virtual ~Cacheable() = default;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_CACHEABLE_LAYER_H_
