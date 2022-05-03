// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_
#define FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_

#include <memory>
#include <optional>

#include "flutter/display_list/display_list_utils.h"
#include "flutter/flow/embedded_views.h"
#include "flutter/flow/raster_cache.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"

namespace flutter {

class DisplayList;
class LayerCacheableItem;
class DisplayListCacheableItem;
class SkPictureCacheableItem;
class CacheableContainerLayer;

class CacheableItem {
 public:
  enum CacheState {
    kNone = 0,
    kCurrent,
    kChildren,
  };

  explicit CacheableItem(RasterCacheKeyID key_id,
                         CacheState cache_state = CacheState::kCurrent,
                         int item_cache_threshold = 1,
                         unsigned child_entries = 0)
      : key_id_(key_id),
        cache_state_(cache_state),
        item_cache_threshold_(item_cache_threshold),
        child_entries_(child_entries) {}

  virtual void PrerollSetup(PrerollContext* context,
                            const SkMatrix& matrix) = 0;

  virtual void PrerollFinalize(PrerollContext* context,
                               const SkMatrix& matrix) = 0;

  virtual bool Draw(const PaintContext& context,
                    const SkPaint* paint) const = 0;

  void Touch(const RasterCache*);

  virtual std::optional<RasterCacheKeyID> GetId() const { return key_id_; }

  virtual LayerCacheableItem* asLayerCacheableItem() { return nullptr; }

  virtual DisplayListCacheableItem* asDisplayCacheableItem() { return nullptr; }

  virtual SkPictureCacheableItem* asSkPictureCacheableItem() { return nullptr; }

  virtual bool TryToPrepareRasterCache(const PaintContext& context) const = 0;

  unsigned chld_entries() const { return child_entries_; }

  void set_matrix(const SkMatrix& matrix) { matrix_ = matrix; }

  void set_child_entries(unsigned child_entries) {
    child_entries_ = child_entries;
  }

  bool need_cached() const { return cache_state_ != CacheState::kNone; }

  virtual ~CacheableItem() = default;

 protected:
  // The id for cache the layer self.
  RasterCacheKeyID key_id_;
  CacheState cache_state_ = CacheState::kCurrent;
  int item_cache_threshold_ = 1;
  int num_cache_attempts_ = 0;
  mutable SkMatrix matrix_;
  unsigned child_entries_;
};

class LayerCacheableItem : public CacheableItem {
 public:
  explicit LayerCacheableItem(Layer* layer, int layer_cached_threshold = 1);

  LayerCacheableItem* asLayerCacheableItem() override { return this; }

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

class DisplayListCacheableItem : public CacheableItem {
 public:
  DisplayListCacheableItem(DisplayList* display_list,
                           const SkPoint& offset,
                           bool is_complex = true,
                           bool will_change = false);

  DisplayListCacheableItem* asDisplayCacheableItem() override { return this; }

  void PrerollSetup(PrerollContext* context, const SkMatrix& matrix) override;

  void PrerollFinalize(PrerollContext* context,
                       const SkMatrix& matrix) override;

  bool ShouldBeCached(const RasterCache* raster_cache,
                      const GrDirectContext* gr_context) const;

  bool Draw(const PaintContext& context, const SkPaint* paint) const override;

  bool TryToPrepareRasterCache(const PaintContext& context) const override;

  void ModifyMatrix(SkPoint offset) const {
    matrix_ = matrix_.preTranslate(offset.x(), offset.y());
  }

  DisplayList* display_list_;
  SkPoint offset_;
  bool is_complex_;
  bool will_change_;
};

class SkPictureCacheableItem : public CacheableItem {
 public:
  SkPictureCacheableItem(SkPicture* sk_picture,
                         const SkPoint& offset,
                         bool is_complex = true,
                         bool will_change = false);

  SkPictureCacheableItem* asSkPictureCacheableItem() override { return this; }

  void PrerollSetup(PrerollContext* context, const SkMatrix& matrix) override;

  void PrerollFinalize(PrerollContext* context,
                       const SkMatrix& matrix) override;

  bool Draw(const PaintContext& context, const SkPaint* paint) const override;

  bool ShouldBeCached(const RasterCache* raster_cache) const;

  bool TryToPrepareRasterCache(const PaintContext& context) const override;

  void ModifyMatrix(SkPoint offset) const {
    matrix_ = matrix_.preTranslate(offset.x(), offset.y());
  }

  SkPicture* sk_picture_;
  SkPoint offset_;
  bool is_complex_;
  bool will_change_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_
