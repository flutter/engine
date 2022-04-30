// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_
#define FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_

#include <memory>

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

class CacheableItem {
 public:
  explicit CacheableItem(const SkMatrix& matrix = SkMatrix::I(),
                         unsigned child_entries = 0,
                         bool need_cached = true)
      : matrix_(matrix),
        child_entries_(child_entries),
        need_cached_(need_cached) {}

  virtual bool Prepare(PaintContext* paint_context) const;

  virtual bool Touch(const RasterCache* raster_cache,
                     bool parent_cached = false) const;

  virtual bool Draw(const RasterCache* raster_cache,
                    SkCanvas& canvas,
                    const SkPaint* paint = nullptr) const;

  virtual std::optional<RasterCacheKey> GetKey(
      SkCanvas* canvas = nullptr) const = 0;

  virtual std::unique_ptr<RasterCacheResult> CreateRasterCache(
      PaintContext* paint_context,
      bool checkerboard = false) const = 0;

  virtual LayerCacheableItem* asLayerCacheableItem() { return nullptr; }

  virtual DisplayListCacheableItem* asDisplayCacheableItem() { return nullptr; }

  virtual SkPictureCacheableItem* asSkPictureCacheableItem() { return nullptr; }

  virtual bool TryToPrepareRasterCache(PaintContext* context) const = 0;

  void Reset();

  bool need_cached() const { return need_cached_; }

  unsigned chld_entries() const { return child_entries_; }

  void set_cull_rect(const SkRect& cull_rect) { cull_rect_ = cull_rect; }

  void set_matrix(const SkMatrix& matrix) { matrix_ = matrix; }

  void set_child_entries(unsigned child_entries) {
    child_entries_ = child_entries;
  }

  void set_need_cached(bool need_cached) { need_cached_ = need_cached; }

  virtual ~CacheableItem() = default;

 protected:
  mutable SkMatrix matrix_;
  mutable SkRect cull_rect_;
  unsigned child_entries_;
  bool need_cached_;
};

class LayerCacheableItem : public CacheableItem {
 public:
  explicit LayerCacheableItem(Layer* layer,
                              const SkMatrix& matrix = SkMatrix::I());

  void set_cache_layer() const { strategy_ = RasterCacheLayerStrategy::kLayer; }

  void set_cache_children_layer() const {
    strategy_ = RasterCacheLayerStrategy::kLayerChildren;
  }

  RasterCacheLayerStrategy GetStrategy() const { return strategy_; }

  LayerCacheableItem* asLayerCacheableItem() override { return this; }

  std::optional<RasterCacheKey> GetKey(
      SkCanvas* canvas = nullptr) const override;

  std::unique_ptr<RasterCacheResult> CreateRasterCache(
      PaintContext* paint_context,
      bool checkerboard = false) const override;

  bool TryToPrepareRasterCache(PaintContext* context) const override;

  void SetHasPlatformView(bool has_platform_view) {
    has_platform_view_ = has_platform_view;
  }

  void SetHasTextureLayer(bool has_texture_layer) {
    has_texture_layer_ = has_texture_layer;
  }

 protected:
  std::optional<RasterCacheKey> TryToMakeRasterCacheKeyForLayer(
      RasterCacheLayerStrategy strategy,
      const SkMatrix& ctm) const;

  const SkRect& GetPaintBoundsFromLayer() const;

  bool has_platform_view_;
  bool has_texture_layer_;

  Layer* layer_;

  mutable RasterCacheLayerStrategy strategy_ = RasterCacheLayerStrategy::kLayer;
};

class DisplayListCacheableItem : public CacheableItem {
 public:
  DisplayListCacheableItem(DisplayList* display_list,
                           const SkRect& bounds,
                           const SkMatrix& matrix = SkMatrix::I(),
                           bool is_complex = true,
                           bool will_change = false);

  DisplayListCacheableItem* asDisplayCacheableItem() override { return this; }

  bool Prepare(PaintContext* paint_context) const override;

  bool ShouldBeCached(const RasterCache* raster_cache,
                      const GrDirectContext* gr_context) const;

  bool TryToPrepareRasterCache(PaintContext* context) const override;

  std::optional<RasterCacheKey> GetKey(
      SkCanvas* canvas = nullptr) const override;

  std::unique_ptr<RasterCacheResult> CreateRasterCache(
      PaintContext* paint_context,
      bool checkerboard = false) const override;

  void ModifyMatrix(SkPoint offset) const {
    matrix_ = matrix_.preTranslate(offset.x(), offset.y());
  }

  DisplayList* display_list_;
  SkRect bounds_;
  bool is_complex_;
  bool will_change_;
};

class SkPictureCacheableItem : public CacheableItem {
 public:
  SkPictureCacheableItem(SkPicture* sk_picture,
                         const SkRect& bounds,
                         const SkMatrix& matrix = SkMatrix::I(),
                         bool is_complex = true,
                         bool will_change = false);

  SkPictureCacheableItem* asSkPictureCacheableItem() override { return this; }

  bool Prepare(PaintContext* paint_context) const override;

  bool ShouldBeCached(const RasterCache* raster_cache) const;

  bool TryToPrepareRasterCache(PaintContext* context) const override;

  std::optional<RasterCacheKey> GetKey(
      SkCanvas* canvas = nullptr) const override;

  std::unique_ptr<RasterCacheResult> CreateRasterCache(
      PaintContext* paint_context,
      bool checkerboard = false) const override;

  void ModifyMatrix(SkPoint offset) const {
    matrix_ = matrix_.preTranslate(offset.x(), offset.y());
  }

  SkPicture* sk_picture_;
  SkRect bounds_;
  bool is_complex_;
  bool will_change_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_
