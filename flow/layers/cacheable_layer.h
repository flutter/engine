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
#include "include/core/SkRect.h"

namespace flutter {

class Cacheable {
 public:
  Cacheable() = default;

  void InitialCacheableLayerItem(Layer*);

  void InitialCacheableDisplayListItem(DisplayList*,
                                       SkRect bounds,
                                       bool is_complex,
                                       bool will_change);

  void InitialCacheableSkPictureItem(SkPicture*,
                                     SkRect bounds,
                                     bool is_complex,
                                     bool will_change);

  class AutoCache {
   public:
    static AutoCache Create(Cacheable* cacheable,
                            PrerollContext* context,
                            const SkMatrix& matrix);

    static AutoCache Create(DisplayListLayer* display_list,
                            PrerollContext* context,
                            const SkMatrix& matrix,
                            SkRect bounds);

    static AutoCache Create(PictureLayer* display_list,
                            PrerollContext* context,
                            const SkMatrix& matrix,
                            SkRect bounds);

    ~AutoCache();

   private:
    AutoCache(Cacheable* cacheable,
              PrerollContext* context,
              const SkMatrix& matrix,
              bool skip = false)
        : cacheable_(cacheable),
          context_(context),
          matrix_(matrix),
          skip_(skip) {
      if (context_ && context_->raster_cache) {
        current_index_ = context_->raster_cached_entries->size();
      }
    }

    int current_index_;
    Cacheable* cacheable_ = nullptr;
    PrerollContext* context_ = nullptr;
    const SkMatrix& matrix_;
    bool skip_ = false;
  };

  virtual Layer* asLayer() = 0;

  virtual void TryToCache(PrerollContext* context, const SkMatrix& ctm) = 0;

  virtual ~Cacheable() = default;

  LayerCacheableItem* GetCacheableLayer() {
    return cacheable_item_.get()->asLayerCacheableItem();
  }

  const LayerCacheableItem* GetCacheableLayer() const {
    return cacheable_item_.get()->asLayerCacheableItem();
  }

  DisplayListCacheableItem* GetCacheableDisplayListItem() {
    return cacheable_item_.get()->asDisplayCacheableItem();
  }

  const DisplayListCacheableItem* GetCacheableDisplayListItem() const {
    return cacheable_item_.get()->asDisplayCacheableItem();
  }

  SkPictureCacheableItem* GetCacheableSkPictureItem() {
    return cacheable_item_.get()->asSkPictureCacheableItem();
  }

  const SkPictureCacheableItem* GetCacheableSkPictureItem() const {
    return cacheable_item_.get()->asSkPictureCacheableItem();
  }

 protected:
  const CacheableItem* GetCacheableItem() const {
    return cacheable_item_.get();
  }

  std::unique_ptr<CacheableItem> cacheable_item_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_CACHEABLE_LAYER_H_
