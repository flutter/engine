// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_UTIL_H_
#define FLUTTER_FLOW_RASTER_CACHE_UTIL_H_

#include "flutter/display_list/geometry/dl_rect.h"
#include "flutter/display_list/geometry/dl_transform.h"
#include "flutter/fml/logging.h"

namespace flutter {

struct RasterCacheUtil {
  // The default max number of picture and display list raster caches to be
  // generated per frame. Generating too many caches in one frame may cause jank
  // on that frame. This limit allows us to throttle the cache and distribute
  // the work across multiple frames.
  static constexpr int kDefaultPictureAndDisplayListCacheLimitPerFrame = 3;

  // The ImageFilterLayer might cache the filtered output of this layer
  // if the layer remains stable (if it is not animating for instance).
  // If the ImageFilterLayer is not the same between rendered frames,
  // though, it will cache its children instead and filter their cached
  // output on the fly.
  // Caching just the children saves the time to render them and also
  // avoids a rendering surface switch to draw them.
  // Caching the layer itself avoids all of that and additionally avoids
  // the cost of applying the filter, but can be worse than caching the
  // children if the filter itself is not stable from frame to frame.
  // This constant controls how many times we will Preroll and Paint this
  // same ImageFilterLayer before we consider the layer and filter to be
  // stable enough to switch from caching the children to caching the
  // filtered output of this layer.
  static constexpr int kMinimumRendersBeforeCachingFilterLayer = 3;

  static bool CanRasterizeRect(const DlFRect& cull_rect) {
    if (cull_rect.IsEmpty()) {
      // No point in ever rasterizing an empty display list.
      return false;
    }

    if (!cull_rect.IsFinite()) {
      // Cannot attempt to rasterize into an infinitely large surface.
      FML_LOG(INFO) << "Attempted to raster cache non-finite display list";
      return false;
    }

    return true;
  }

  static DlFRect GetDeviceBounds(const DlFRect& rect, const DlTransform& ctm) {
    return ctm.TransformRect(rect);
  }

  static DlFRect GetRoundedOutDeviceBounds(const DlFRect& rect,
                                           const DlTransform& ctm) {
    return ctm.TransformRect(rect).RoundedOut();
  }
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHE_UTIL_H_
