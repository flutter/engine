// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_
#define FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_

#include <memory>

#include "flutter/flow/embedded_views.h"
#include "include/core/SkPicture.h"

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
  virtual void TryToPrepareRasterCache(PrerollContext* context,
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

  void TryToPrepareRasterCache(PrerollContext* context,
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
  explicit SkGPUObjectCacheableWrapper(SkPoint offset) : offset_(offset) {}

 protected:
  SkPoint offset_;
};

class CacheableDisplayListWrapper : public SkGPUObjectCacheableWrapper {
 public:
  CacheableDisplayListWrapper(DisplayList* display_list, SkPoint offset)
      : SkGPUObjectCacheableWrapper(offset), display_list_(display_list) {}

  void TryToPrepareRasterCache(PrerollContext* context,
                               const SkMatrix& matrix) override;

  CacheableDisplayListWrapper* asCacheableDisplayListWrapper() override {
    return this;
  }

 private:
  DisplayList* display_list_;
};

class CacheableSkPictureWrapper : public SkGPUObjectCacheableWrapper {
 public:
  CacheableSkPictureWrapper(SkPicture* sk_picture, SkPoint offset)
      : SkGPUObjectCacheableWrapper(offset), sk_picture_(sk_picture) {}

  void TryToPrepareRasterCache(PrerollContext* context,
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

  static std::shared_ptr<RasterCacheableEntry> MarkLayerCacheable(
      Cacheable* layer,
      const PrerollContext& context,
      const SkMatrix& matrix,
      unsigned num_child = 0,
      bool need_caching = true) {
    return std::make_shared<RasterCacheableEntry>(
        std::make_unique<CacheableLayerWrapper>(layer), context, matrix,
        num_child, need_caching);
  }

  static std::shared_ptr<RasterCacheableEntry> MarkDisplayListCacheable(
      DisplayList* display_list,
      const PrerollContext& context,
      const SkMatrix& matrix,
      SkPoint offset,
      unsigned num_child = 0,
      bool need_caching = true) {
    return std::make_shared<RasterCacheableEntry>(
        std::make_unique<CacheableDisplayListWrapper>(display_list, offset),
        context, matrix, num_child, need_caching);
  }

  static std::shared_ptr<RasterCacheableEntry> MarkSkPictureCacheable(
      SkPicture* picture,
      const PrerollContext& context,
      const SkMatrix& matrix,
      SkPoint offset,
      unsigned num_child = 0,
      bool need_caching = true) {
    return std::make_shared<RasterCacheableEntry>(
        std::make_unique<CacheableSkPictureWrapper>(picture, offset), context,
        matrix, num_child, need_caching);
  }

  CacheableItemWrapperBase* GetCacheableWrapper() const { return item_.get(); }

  void MarkNotCache() { need_caching = false; }

  void MarkLayerChildrenNeedCached() {
    item_->asCacheableLayerWrapper()->NeedCacheChildren();
  }

  void MarkTouchCache() { item_->TouchCache(); }

  void TryToPrepareRasterCache(PrerollContext* context);

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
