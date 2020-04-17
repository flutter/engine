// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer.h"
#include "flutter/testing/mock_raster_cache.h"

namespace flutter {
namespace testing {

// RasterCacheResult::RasterCacheResult(sk_sp<SkImage> image,
//                                      const SkRect& logical_rect)
//     : image_(std::move(image)), logical_rect_(logical_rect) {}

// void RasterCacheResult::draw(SkCanvas& canvas, const SkPaint* paint) const {
//   TRACE_EVENT0("flutter", "RasterCacheResult::draw");
//   SkAutoCanvasRestore auto_restore(&canvas, true);
//   SkIRect bounds =
//       RasterCache::GetDeviceBounds(logical_rect_, canvas.getTotalMatrix());
//   FML_DCHECK(
//       std::abs(bounds.size().width() - image_->dimensions().width()) <= 1 &&
//       std::abs(bounds.size().height() - image_->dimensions().height()) <= 1);
//   canvas.resetMatrix();
//   canvas.drawImage(image_, bounds.fLeft, bounds.fTop, paint);
// }

MockRasterCache::MockRasterCache()
    : RasterCache::RasterCache(1, 1000000) { }

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

/// @note Procedure doesn't copy all closures.
static RasterCacheResult Rasterize(
    const SkMatrix& ctm,
    SkColorSpace* dst_color_space,
    const SkRect& logical_rect) {
  TRACE_EVENT0("flutter", "RasterCachePopulate");
  SkIRect cache_rect = RasterCache::GetDeviceBounds(logical_rect, ctm);

  const SkImageInfo image_info = SkImageInfo::MakeN32Premul(
      cache_rect.width(), cache_rect.height(), sk_ref_sp(dst_color_space));

  sk_sp<SkData> data = SkData::MakeUninitialized(image_info.computeMinByteSize());
  sk_sp<SkImage> image = SkImage::MakeRasterData(image_info, data, image_info.minRowBytes());

  return {image, logical_rect};
}

static RasterCacheResult RasterizePicture(SkPicture* picture,
                                   const SkMatrix& ctm,
                                   SkColorSpace* dst_color_space) {
  return Rasterize(ctm, dst_color_space, picture->cullRect());
}

void MockRasterCache::Prepare(PrerollContext* context,
                              Layer* layer,
                              const SkMatrix& ctm) {
  LayerRasterCacheKey cache_key(layer->unique_id(), ctm);
  Entry& entry = layer_cache_[cache_key];
  entry.access_count++;
  entry.used_this_frame = true;
  if (!entry.image.is_valid()) {
    entry.image = Rasterize(
        ctm, context->dst_color_space,
        layer->paint_bounds());
  }
}

bool MockRasterCache::WasPrepared(Layer* layer, const SkMatrix& ctm) {
  LayerRasterCacheKey cache_key(layer->unique_id(), ctm);
  // return layer_cache_.contains(cache_key); - Requires STD_VER > 17, C++20
  return layer_cache_.find(cache_key) != layer_cache_.end();
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
  Entry& entry = picture_cache_[cache_key];

  if (!entry.image.is_valid()) {
    entry.image = RasterizePicture(picture, transformation_matrix,
                                   dst_color_space);
  }
  return true;
}

RasterCacheResult MockRasterCache::Get(const SkPicture& picture,
                                   const SkMatrix& ctm) const {
  PictureRasterCacheKey cache_key(picture.uniqueID(), ctm);
  auto it = picture_cache_.find(cache_key);
  if (it == picture_cache_.end()) {
    return RasterCacheResult();
  }

  Entry& entry = it->second;
  entry.access_count++;
  entry.used_this_frame = true;

  return entry.image;
}

RasterCacheResult MockRasterCache::Get(Layer* layer, const SkMatrix& ctm) const {
  LayerRasterCacheKey cache_key(layer->unique_id(), ctm);
  auto it = layer_cache_.find(cache_key);
  if (it == layer_cache_.end()) {
    return RasterCacheResult();
  }

  Entry& entry = it->second;
  entry.access_count++;
  entry.used_this_frame = true;

  return entry.image;
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

void MockRasterCache::SetCheckboardCacheImages(bool checkerboard) {
}

}  // namespace testing
}  // namespace flutter
