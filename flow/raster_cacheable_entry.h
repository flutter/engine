// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_
#define FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_

#include <memory>

#include "flutter/flow/embedded_views.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"

namespace flutter {

struct PrerollContext;
class RasterCache;
class DisplayList;
class Cacheable;
class CacheableLayerWrapper;
class CacheableDisplayListWrapper;
class CacheableSkPictureWrapper;

class CacheableItemWrapperBase {
 public:
  virtual bool TryToPrepareRasterCache(PrerollContext* context,
                                       const SkMatrix& matrix) = 0;

  virtual void TouchRasterCache(PrerollContext* context,
                                const SkMatrix& matrix) = 0;

  virtual CacheableLayerWrapper* asCacheableLayerWrapper() { return nullptr; }

  virtual CacheableDisplayListWrapper* asCacheableDisplayListWrapper() {
    return nullptr;
  }

  virtual CacheableSkPictureWrapper* asCacheableSkPictureWrapper() {
    return nullptr;
  }

  virtual ~CacheableItemWrapperBase() = default;

  void TouchCache() { touch_cache_ = true; }

 protected:
  bool touch_cache_ = false;
};
// CacheableEntry is a wrapper to erasure the Entry type.
class CacheableLayerWrapper : public CacheableItemWrapperBase {
 public:
  explicit CacheableLayerWrapper(Cacheable* layer) : cacheable_item_(layer) {}

  bool TryToPrepareRasterCache(PrerollContext* context,
                               const SkMatrix& matrix) override;

  void TouchRasterCache(PrerollContext* context,
                        const SkMatrix& matrix) override;

  void NeedCacheChildren() { cache_children_ = true; }

  CacheableLayerWrapper* asCacheableLayerWrapper() override { return this; }

  Cacheable* GetCacheableItem() const { return cacheable_item_; }

 private:
  bool cache_children_ = false;
  Cacheable* cacheable_item_;
};

// CacheableEntry is a wrapper to erasure the Entry type.
class SkGPUObjectCacheableWrapper : public CacheableItemWrapperBase {
 public:
  explicit SkGPUObjectCacheableWrapper(SkRect offset) : bounds_(offset) {}

 protected:
  SkRect bounds_;
};

class CacheableDisplayListWrapper : public SkGPUObjectCacheableWrapper {
 public:
  CacheableDisplayListWrapper(DisplayList* display_list, SkRect bounds_)
      : SkGPUObjectCacheableWrapper(bounds_), display_list_(display_list) {}

  bool TryToPrepareRasterCache(PrerollContext* context,
                               const SkMatrix& matrix) override;

  void TouchRasterCache(PrerollContext* context,
                        const SkMatrix& matrix) override;

  CacheableDisplayListWrapper* asCacheableDisplayListWrapper() override {
    return this;
  }

 private:
  DisplayList* display_list_;
};

class CacheableSkPictureWrapper : public SkGPUObjectCacheableWrapper {
 public:
  CacheableSkPictureWrapper(SkPicture* sk_picture, SkRect bounds)
      : SkGPUObjectCacheableWrapper(bounds), sk_picture_(sk_picture) {}

  bool TryToPrepareRasterCache(PrerollContext* context,
                               const SkMatrix& matrix) override;

  void TouchRasterCache(PrerollContext* context,
                        const SkMatrix& matrix) override;

  CacheableSkPictureWrapper* asCacheableSkPictureWrapper() override {
    return this;
  }

 private:
  SkPicture* sk_picture_;
};

// A class used for collection the entry which can be raster cached.
// The entry may be a Layer, DisplayList, or SkPicture
class RasterCacheableEntry {
 public:
  RasterCacheableEntry(std::unique_ptr<CacheableItemWrapperBase> item,
                       const PrerollContext& context,
                       const SkMatrix& matrix,
                       unsigned num_child,
                       bool need_caching = true);

  /// Create a layer entry.
  /// The entry may be null if the PrerollContext's raster_cache is null
  static std::shared_ptr<RasterCacheableEntry> MakeLayerCacheable(
      Cacheable* layer,
      const PrerollContext& context,
      const SkMatrix& matrix,
      unsigned num_child = 0,
      bool need_caching = true);

  /// Create a display_list entry.
  /// The entry may be null if the PrerollContext's raster_cache is null
  static std::shared_ptr<RasterCacheableEntry> MakeDisplayListCacheable(
      DisplayList* display_list,
      const PrerollContext& context,
      const SkMatrix& matrix,
      SkRect bounds,
      unsigned num_child = 0,
      bool need_caching = true);

  /// Create a sk_picture entry.
  /// The entry may be null if the PrerollContext's raster_cache is null
  static std::shared_ptr<RasterCacheableEntry> MakeSkPictureCacheable(
      SkPicture* picture,
      const PrerollContext& context,
      const SkMatrix& matrix,
      SkRect bounds,
      unsigned num_child = 0,
      bool need_caching = true);

  CacheableItemWrapperBase* GetCacheableWrapper() const { return item_.get(); }

  PrerollContext MakeEntryPrerollContext(PrerollContext* context);

  void MarkNotCache() { need_caching = false; }

  void MarkLayerChildrenNeedCached() {
    item_->asCacheableLayerWrapper()->NeedCacheChildren();
  }

  void MarkTouchCache() { item_->TouchCache(); }

  bool TryToPrepareRasterCache(PrerollContext* context) {
    return item_->TryToPrepareRasterCache(context, matrix);
  }

  void TouchRasterCache(PrerollContext* context) {
    item_->TouchRasterCache(context, matrix);
  }

  Cacheable* GetCacheableLayer() {
    return item_->asCacheableLayerWrapper()->GetCacheableItem();
  }

  SkMatrix matrix;
  SkRect cull_rect;

  unsigned num_child_entries;
  bool need_caching;

  bool has_platform_view = false;
  bool has_texture_layer = false;

 private:
  std::unique_ptr<CacheableItemWrapperBase> item_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_
