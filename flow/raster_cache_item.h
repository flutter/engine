// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_ITEM_H_
#define FLUTTER_FLOW_RASTER_CACHE_ITEM_H_

#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache_key.h"

class SkColorSpace;

namespace flutter {

class RasterCacheItem {
 public:
  // The default max number of picture and display list raster caches to be
  // generated per frame. Generating too many caches in one frame may cause jank
  // on that frame. This limit allows us to throttle the cache and distribute
  // the work across multiple frames.
  // static constexpr int kDefaultPictureAndDispLayListCacheLimitPerFrame = 3;

  RasterCacheItem() = default;

  virtual ~RasterCacheItem() = default;

  virtual void PrerollSetup(PrerollContext* context,
                            const SkMatrix& matrix) = 0;
  virtual void PrerollFinalize(PrerollContext* context,
                               const SkMatrix& matrix) = 0;

  virtual int itemChildren() = 0;

  virtual bool PrepareForFrame(const Layer::PaintContext& p_context,
                               RasterCache* cache,
                               const RasterCache::Context& r_context,
                               bool parent_cached) const = 0;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(RasterCacheItem);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHE_ITEM_H_
