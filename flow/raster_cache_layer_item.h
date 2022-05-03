// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_LAYER_ITEM_H_
#define FLUTTER_FLOW_RASTER_CACHE_LAYER_ITEM_H_

#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/raster_cache_item.h"

namespace flutter {

class CacheableContainerLayer : public ContainerLayer {
 public:
  virtual bool canCacheChildren(PrerollContext* context,
                                const SkMatrix& matrix) = 0;

  virtual void updatePaintForLayer(AutoCachePaint& paint) {}
  virtual void updatePaintForChildren(AutoCachePaint& paint) {}
};

enum class RasterCacheLayerStrategy { kLayer, kLayerChildren };

/// This class implements the RasterCacheItem flow for layers
/// that filter their children in some way and may want to
/// cache their children to apply a filter to them as a bitmap
/// operation, or to cache the filtered result of the final
/// rendering of the layer.
///
/// Layers that plan to use this cache helper class should
/// implement the |CacheableContainerLayer| class and override
/// those methods to produce appropriate responses.
class RasterCacheLayerItem : public RasterCacheItem {
 public:
  explicit RasterCacheLayerItem(CacheableContainerLayer* layer,
                                int layer_cache_thresholds);

  virtual ~RasterCacheLayerItem() = default;

  void PrerollSetup(PrerollContext* context, const SkMatrix& matrix) override;
  void PrerollFinalize(PrerollContext* context,
                       const SkMatrix& matrix) override;

  int itemChildren() override { return num_children_cached_; };

  bool PrepareForFrame(const Layer::PaintContext& p_context,
                       RasterCache* cache,
                       const RasterCache::Context& r_context,
                       bool parent_cached) const override;
  bool RasterizeLayer(const Layer::PaintContext& context,
                      SkCanvas* canvas) const;

  bool Draw(const Layer::PaintContext& context,
            ContainerLayer::AutoCachePaint& paint) const;

 private:
  CacheableContainerLayer* layer_;
  int layer_cache_threshold_;
  RasterCacheKeyID layer_key_id_;
  std::optional<RasterCacheKeyID> layer_children_id_;

  std::function<bool(PrerollContext* context, const SkMatrix& matrix)>
      children_cacheable_function_;

  static bool default_children_cacheable_(PrerollContext* context,
                                          const SkMatrix& matrix) {
    return true;
  }

  enum class CacheState {
    kDoNotCache,
    kCacheAsLayer,
    kCacheChildren,
  };

  const std::optional<RasterCacheKeyID> id_() const;

  SkMatrix cache_matrix_;
  int num_children_cached_;
  int num_cache_attempts_;
  CacheState cache_state_;

  FML_DISALLOW_COPY_AND_ASSIGN(RasterCacheLayerItem);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHE_LAYER_ITEM_H_
