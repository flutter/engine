// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/mock_raster_cache.h"
#include "flutter/flow/layers/layer.h"

namespace flutter {
namespace testing {

MockRasterCache::MockRasterCache() : RasterCache::RasterCache(1, 1000000) {}

static bool CanRasterizePicture(SkPicture* picture) {
  if (picture == nullptr) {
    return false;
  }

  const SkRect cull_rect = picture->cullRect();

  if (cull_rect.isEmpty()) {
    // No point in ever rasterizing an empty picture.
    return false;
  }

  if (!cull_rect.isFinite()) {
    // Cannot attempt to rasterize into an infinitely large surface.
    return false;
  }

  return true;
}

static bool IsPictureWorthRasterizing(SkPicture* picture,
                                      bool will_change,
                                      bool is_complex) {
  if (will_change) {
    // If the picture is going to change in the future, there is no point in
    // doing to extra work to rasterize.
    return false;
  }

  if (!CanRasterizePicture(picture)) {
    // No point in deciding whether the picture is worth rasterizing if it
    // cannot be rasterized at all.
    return false;
  }

  if (is_complex) {
    // The caller seems to have extra information about the picture and thinks
    // the picture is always worth rasterizing.
    return true;
  }

  // TODO(abarth): We should find a better heuristic here that lets us avoid
  // wasting memory on trivial layers that are easy to re-rasterize every frame.
  return picture->approximateOpCount() > 5;
}

void MockRasterCache::Prepare(PrerollContext* context,
                              Layer* layer,
                              const SkMatrix& ctm) {
  LayerRasterCacheKey cache_key(layer->unique_id(), ctm);
  MockEntry& entry = layer_cache_[cache_key];
  entry.access_count++;
  entry.used_this_frame = true;
  entry.rasterized = true;
}

bool MockRasterCache::Prepare(GrContext* context,
                              SkPicture* picture,
                              const SkMatrix& transformation_matrix,
                              SkColorSpace* dst_color_space,
                              bool is_complex,
                              bool will_change) {
  if (!IsPictureWorthRasterizing(picture, will_change, is_complex)) {
    // We only deal with pictures that are worthy of rasterization.
    return false;
  }

  // Decompose the matrix (once) for all subsequent operations. We want to make
  // sure to avoid volumetric distortions while accounting for scaling.
  const MatrixDecomposition matrix(transformation_matrix);

  if (!matrix.IsValid()) {
    // The matrix was singular. No point in going further.
    return false;
  }

  PictureRasterCacheKey cache_key(picture->uniqueID(), transformation_matrix);

  // Creates an entry, if not present prior.
  MockEntry& entry = picture_cache_[cache_key];
  entry.rasterized = true;

  return true;
}

bool MockRasterCache::Draw(const SkPicture& picture, SkCanvas& canvas) const {
  PictureRasterCacheKey cache_key(picture.uniqueID(), canvas.getTotalMatrix());
  auto it = picture_cache_.find(cache_key);
  if (it == picture_cache_.end()) {
    return false;
  }

  MockEntry& entry = it->second;
  entry.access_count++;
  entry.used_this_frame = true;

  return entry.rasterized;
}

bool MockRasterCache::Draw(const Layer* layer,
                           SkCanvas& canvas,
                           SkPaint* paint) const {
  LayerRasterCacheKey cache_key(layer->unique_id(), canvas.getTotalMatrix());
  auto it = layer_cache_.find(cache_key);
  if (it == layer_cache_.end()) {
    return false;
  }

  MockEntry& entry = it->second;
  entry.access_count++;
  entry.used_this_frame = true;

  return entry.rasterized;
}

void MockRasterCache::SweepAfterFrame() {
  SweepOneCacheAfterFrame(picture_cache_);
  SweepOneCacheAfterFrame(layer_cache_);
}

void MockRasterCache::Clear() {
  picture_cache_.clear();
  layer_cache_.clear();
}

size_t MockRasterCache::GetCachedEntriesCount() const {
  return layer_cache_.size() + picture_cache_.size();
}

void MockRasterCache::SetCheckboardCacheImages(bool checkerboard) {}

}  // namespace testing
}  // namespace flutter
