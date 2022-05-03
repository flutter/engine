// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_ITEM_H_
#define FLUTTER_FLOW_RASTER_CACHE_ITEM_H_

#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache_key.h"

class SkColorSpace;

namespace flutter {

/// A RasterCacheItem is the base class for an object that will handle
/// all of the actions necessary for a layer to participate in the
/// caching of itself, a rendering object it holds, or all of its
/// children.
///
/// Implementations of this class (that may impose their own
/// additional constraints) exist for:
///   RasterCacheLayerItem -
///       helps a filtering layer that wants to cache either itself
///       or its children.
///   RasterCacheDisplayListItem -
///       implements this flow for DisplayList object
///   RasterCachePictureItem - (tbd, similar for SkPicture)
///
/// The basic flow of operations is as follows:
///
/// |PrerollSetup| should be called near the start of the Preroll
/// method any time until the children are Prerolled recursively.
///
/// |PrerollFinalize| should be called near the end of the Preroll
/// method any time after the children are Prerolled recursively.
/// This method should touch the cache entry that it is planning
/// to utilize in its Paint method
///
/// TODO(flar or jsouliang) PrerollSetup/Finalize could be called
/// automatically from some form of "Auto" object.
///
/// |PrepareForFrame| will be called from the code that resolves
/// the list of RasterCacheItems just before |Paint| is called
/// from the layer tree.
///
/// sub classes will typically provide a |Draw| method that
/// navigates the RasterCache methods to draw the indicated
/// cache entry into the destination.
class RasterCacheItem {
 public:
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
