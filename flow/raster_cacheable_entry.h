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
class CacheableLayer;
class DisplayList;
class CacheableLayerWrapper;
class CacheableDisplayListWrapper;
class CacheableSkPictureWrapper;

class CacheableItemWrapperBase {
 public:
  virtual void TryToPrepareRasterCache(PrerollContext* context,
                                       const SkMatrix& matrix) = 0;

  virtual CacheableLayerWrapper* GetCacheableLayer() { return nullptr; }

  virtual CacheableDisplayListWrapper* GetCacheableDisplayList() {
    return nullptr;
  }

  virtual CacheableSkPictureWrapper* GetCacheableSkPicture() { return nullptr; }

  virtual ~CacheableItemWrapperBase() = default;
};
// CacheableEntry is a wrapper to erasure the Entry type.
class CacheableLayerWrapper : public CacheableItemWrapperBase {
 public:
  explicit CacheableLayerWrapper(CacheableLayer* layer) : layer_(layer) {}

  void TryToPrepareRasterCache(PrerollContext* context,
                               const SkMatrix& matrix) override;

  void NeedCacheChildren() { cache_children_ = true; }

  CacheableLayerWrapper* GetCacheableLayer() override { return this; }

 private:
  bool cache_children_ = false;
  CacheableLayer* layer_;
};

// CacheableEntry is a wrapper to erasure the Entry type.
class SkGPUObjectCacheableWrapper : public CacheableItemWrapperBase {
 public:
  explicit SkGPUObjectCacheableWrapper(SkPoint offset,
                                       bool is_complex,
                                       bool will_change)
      : offset_(offset), is_complex_(is_complex), will_change_(will_change) {}

 protected:
  SkPoint offset_;
  bool is_complex_ = false;
  bool will_change_ = false;
};

class CacheableDisplayListWrapper : public SkGPUObjectCacheableWrapper {
 public:
  CacheableDisplayListWrapper(DisplayList* display_list,
                              SkPoint offset,
                              bool is_complex,
                              bool will_change)
      : SkGPUObjectCacheableWrapper(offset, is_complex, will_change),
        display_list_(display_list) {}

  void TryToPrepareRasterCache(PrerollContext* context,
                               const SkMatrix& matrix) override;

  CacheableDisplayListWrapper* GetCacheableDisplayList() override {
    return this;
  }

 private:
  DisplayList* display_list_;
};

class CacheableSkPictureWrapper : public SkGPUObjectCacheableWrapper {
 public:
  CacheableSkPictureWrapper(SkPicture* sk_picture,
                            SkPoint offset,
                            bool is_complex,
                            bool will_change)
      : SkGPUObjectCacheableWrapper(offset, is_complex, will_change),
        sk_picture_(sk_picture) {}

  void TryToPrepareRasterCache(PrerollContext* context,
                               const SkMatrix& matrix) override;

  CacheableSkPictureWrapper* GetCacheableSkPicture() override { return this; }

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
                       bool need_caching = false);

  static std::shared_ptr<RasterCacheableEntry> MarkLayerCacheable(
      CacheableLayer* layer,
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
      bool is_complex = false,
      bool will_change = false,
      unsigned num_child = 0,
      bool need_caching = true) {
    return std::make_shared<RasterCacheableEntry>(
        std::make_unique<CacheableDisplayListWrapper>(display_list, offset,
                                                      is_complex, will_change),
        context, matrix, num_child, need_caching);
  }

  static std::shared_ptr<RasterCacheableEntry> MarkSkPictureCacheable(
      SkPicture* picture,
      const PrerollContext& context,
      const SkMatrix& matrix,
      SkPoint offset,
      bool is_complex = false,
      bool will_change = false,
      unsigned num_child = 0,
      bool need_caching = true) {
    return std::make_shared<RasterCacheableEntry>(
        std::make_unique<CacheableSkPictureWrapper>(picture, offset, is_complex,
                                                    will_change),
        context, matrix, num_child, need_caching);
  }

  CacheableItemWrapperBase* GetCacheableWrapper() const { return item_.get(); }

  void MarkLayerChildrenNeedCached() {
    item_->GetCacheableLayer()->NeedCacheChildren();
  }

  void TryToPrepareRasterCache(PrerollContext* context);

  SkMatrix matrix;
  MutatorsStack mutators_stack;
  SkRect cull_rect;

  unsigned num_child_entries;
  bool need_caching;

 private:
  std::unique_ptr<CacheableItemWrapperBase> item_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHEABLE_ENTRY_H_
