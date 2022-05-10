// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_ITEM_H_
#define FLUTTER_FLOW_RASTER_CACHE_ITEM_H_

#include <memory>
#include <optional>

#include "flutter/flow/raster_cache_key.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"

namespace flutter {

struct PrerollContext;
struct PaintContext;
class DisplayList;
class RasterCache;
class LayerRasterCacheItem;
class DisplayListRasterCacheItem;
class SkPictureRasterCacheItem;

class RasterCacheItem {
 public:
  enum CacheState {
    kNone = 0,
    kCurrent,
    kChildren,
  };

  explicit RasterCacheItem(RasterCacheKeyID key_id,
                           CacheState cache_state = CacheState::kCurrent,
                           unsigned child_entries = 0)
      : key_id_(key_id),
        cache_state_(cache_state),
        child_items_(child_entries) {}

  virtual void PrerollSetup(PrerollContext* context,
                            const SkMatrix& matrix) = 0;

  virtual void PrerollFinalize(PrerollContext* context,
                               const SkMatrix& matrix) = 0;

  virtual bool Draw(const PaintContext& context,
                    const SkPaint* paint) const = 0;

  virtual bool Draw(const PaintContext& context,
                    SkCanvas* canvas,
                    const SkPaint* paint) const = 0;

  /**
   * @brief Create a LayerRasterCacheItem, connect a layer and manage the
   * Layer's raster cache
   *
   * @param layer_cache_threshold  after how many frames to start trying to
   * cache the layer self
   * @param can_cache_children the layer can do a cache for his children
   */
  static std::unique_ptr<LayerRasterCacheItem> MakeLayerRasterCacheItem(
      Layer*,
      int layer_cache_threshold,
      bool can_cache_children = false);

  static std::unique_ptr<DisplayListRasterCacheItem>
  MakeDisplayListRasterCacheItem(DisplayList*,
                                 const SkPoint& offset,
                                 bool is_complex,
                                 bool will_change);

  static std::unique_ptr<SkPictureRasterCacheItem> MakeSkPictureRasterCacheItem(
      SkPicture*,
      const SkPoint& offset,
      bool is_complex,
      bool will_change);

  virtual std::optional<RasterCacheKeyID> GetId() const { return key_id_; }

  virtual bool TryToPrepareRasterCache(const PaintContext& context,
                                       bool parent_cached = false) const = 0;

  unsigned child_items() const { return child_items_; }

  void set_matrix(const SkMatrix& matrix) { matrix_ = matrix; }

  CacheState cache_state() const { return cache_state_; }

  bool need_caching() const { return cache_state_ != CacheState::kNone; }

  virtual ~RasterCacheItem() = default;

 protected:
  // The id for cache the layer self.
  RasterCacheKeyID key_id_;
  CacheState cache_state_ = CacheState::kCurrent;
  mutable int num_cache_attempts_ = 1;
  mutable SkMatrix matrix_;
  unsigned child_items_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHE_ITEM_H_
