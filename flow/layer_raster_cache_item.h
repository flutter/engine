// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYER_RASTER_CACHE_ITEM_H_
#define FLUTTER_FLOW_LAYER_RASTER_CACHE_ITEM_H_

#include <memory>
#include <optional>

#include "flutter/flow/raster_cache_item.h"

namespace flutter {

class LayerRasterCacheItem : public RasterCacheItem {
 public:
  explicit LayerRasterCacheItem(Layer* layer,
                                int layer_cached_threshold = 1,
                                bool can_cache_children = false);

  std::optional<RasterCacheKeyID> GetId() const override;

  void PrerollSetup(PrerollContext* context, const SkMatrix& matrix) override;

  void PrerollFinalize(PrerollContext* context,
                       const SkMatrix& matrix) override;

  bool Draw(const PaintContext& context, const SkPaint* paint) const override;

  bool Rasterize(const PaintContext& paint_context, SkCanvas* canvas) const;

  bool TryToPrepareRasterCache(const PaintContext& context) const override;

  void CacheChildren(const SkMatrix& matrix) {
    matrix_ = matrix;
    can_cache_children_ = true;
  }

  bool IsCacheChildren() const { return cache_state_ == CacheState::kChildren; }

 protected:
  Layer* layer_;

  // The id for cache the layer's children.
  std::optional<RasterCacheKeyID> layer_children_id_;

  std::optional<RasterCacheKey> TryToMakeRasterCacheKeyForLayer(
      RasterCacheLayerStrategy strategy,
      const SkMatrix& ctm) const;

  const SkRect& GetPaintBoundsFromLayer() const;
  // if the layer's children can be directly cache, set the param is true;
  bool can_cache_children_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYER_RASTER_CACHE_ITEM_H_
