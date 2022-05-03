// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_DISPLAY_LIST_ITEM_H_
#define FLUTTER_FLOW_RASTER_CACHE_DISPLAY_LIST_ITEM_H_

#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/raster_cache_item.h"

namespace flutter {

/// This class implements the RasterCacheItem flow for layers
/// that display A DisplayList.
///
/// TODO(flar or jsouliang) implement a Picture version for
/// SkPicture objects, or use this as a base class that can
/// serve either a DisplayList or an SkPicture.
class RasterCacheDisplayListItem : public RasterCacheItem {
 public:
  // The default max number of picture and display list raster caches to be
  // generated per frame. Generating too many caches in one frame may cause jank
  // on that frame. This limit allows us to throttle the cache and distribute
  // the work across multiple frames.
  static constexpr int kDefaultPictureAndDispLayListCacheLimitPerFrame = 3;

  explicit RasterCacheDisplayListItem(
      sk_sp<DisplayList> display_list,
      const SkPoint offset,
      bool is_complex,
      bool will_change,
      int picture_cache_thresholds =
          kDefaultPictureAndDispLayListCacheLimitPerFrame);

  virtual ~RasterCacheDisplayListItem() = default;

  void PrerollSetup(PrerollContext* context, const SkMatrix& matrix) override;
  void PrerollFinalize(PrerollContext* context,
                       const SkMatrix& matrix) override;

  int itemChildren() override { return 0; };

  bool PrepareForFrame(const Layer::PaintContext& p_context,
                       RasterCache* cache,
                       const RasterCache::Context& r_context,
                       bool parent_cached) const override;
  bool RasterizeLayer(const Layer::PaintContext& context,
                      SkCanvas* canvas) const;

  bool Draw(const Layer::PaintContext& context,
            ContainerLayer::AutoCachePaint& paint) const;

 private:
  sk_sp<DisplayList> display_list_;
  SkPoint offset_;
  bool is_complex_;
  bool will_change_;
  int picture_cache_threshold_;
  RasterCacheKeyID display_list_key_id_;

  SkMatrix cache_matrix_;
  int num_cache_attempts_;
  bool cacheable_;

  FML_DISALLOW_COPY_AND_ASSIGN(RasterCacheDisplayListItem);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHE_DISPLAY_LIST_ITEM_H_
