// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/raster_cache.h"

#include <vector>

#include "flutter/common/constants.h"
#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/paint_utils.h"
#include "flutter/flow/raster_cacheable_entry.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

RasterCacheResult::RasterCacheResult(sk_sp<SkImage> image,
                                     const SkRect& logical_rect,
                                     const char* type)
    : image_(std::move(image)), logical_rect_(logical_rect), flow_(type) {}

void RasterCacheResult::draw(SkCanvas& canvas, const SkPaint* paint) const {
  TRACE_EVENT0("flutter", "RasterCacheResult::draw");
  SkAutoCanvasRestore auto_restore(&canvas, true);

  SkRect bounds =
      RasterCache::GetDeviceBounds(logical_rect_, canvas.getTotalMatrix());
  FML_DCHECK(std::abs(bounds.width() - image_->dimensions().width()) <= 1 &&
             std::abs(bounds.height() - image_->dimensions().height()) <= 1);
  canvas.resetMatrix();
  flow_.Step();
  canvas.drawImage(image_, bounds.fLeft, bounds.fTop, SkSamplingOptions(),
                   paint);
}

RasterCache::RasterCache(size_t access_threshold,
                         size_t picture_and_display_list_cache_limit_per_frame)
    : access_threshold_(access_threshold),
      picture_and_display_list_cache_limit_per_frame_(
          picture_and_display_list_cache_limit_per_frame),
      checkerboard_images_(false) {}

static bool CanRasterizeRect(const SkRect& cull_rect) {
  if (cull_rect.isEmpty()) {
    // No point in ever rasterizing an empty display list.
    return false;
  }

  if (!cull_rect.isFinite()) {
    // Cannot attempt to rasterize into an infinitely large surface.
    FML_LOG(INFO) << "Attempted to raster cache non-finite display list";
    return false;
  }

  return true;
}

bool RasterCache::IsPictureWorthRasterizing(SkPicture* picture,
                                            bool will_change,
                                            bool is_complex) {
  if (will_change) {
    // If the picture is going to change in the future, there is no point in
    // doing to extra work to rasterize.
    return false;
  }

  if (picture == nullptr || !CanRasterizeRect(picture->cullRect())) {
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
  return picture->approximateOpCount(true) > 5;
}

bool RasterCache::IsDisplayListWorthRasterizing(
    DisplayList* display_list,
    bool will_change,
    bool is_complex,
    DisplayListComplexityCalculator* complexity_calculator) {
  if (will_change) {
    // If the display list is going to change in the future, there is no point
    // in doing to extra work to rasterize.
    return false;
  }

  if (display_list == nullptr || !CanRasterizeRect(display_list->bounds())) {
    // No point in deciding whether the display list is worth rasterizing if it
    // cannot be rasterized at all.
    return false;
  }

  if (is_complex) {
    // The caller seems to have extra information about the display list and
    // thinks the display list is always worth rasterizing.
    return true;
  }

  unsigned int complexity_score = complexity_calculator->Compute(display_list);
  return complexity_calculator->ShouldBeCached(complexity_score);
}

/// @note Procedure doesn't copy all closures.
std::unique_ptr<RasterCacheResult> RasterCache::Rasterize(
    GrDirectContext* context,
    const SkMatrix& ctm,
    SkColorSpace* dst_color_space,
    bool checkerboard,
    const SkRect& logical_rect,
    const char* type,
    const std::function<void(SkCanvas*)>& draw_function) {
  TRACE_EVENT0("flutter", "RasterCachePopulate");

  SkRect dest_rect = RasterCache::GetDeviceBounds(logical_rect, ctm);
  // we always round out here so that the texture is integer sized.
  int width = SkScalarCeilToInt(dest_rect.width());
  int height = SkScalarCeilToInt(dest_rect.height());

  const SkImageInfo image_info =
      SkImageInfo::MakeN32Premul(width, height, sk_ref_sp(dst_color_space));

  sk_sp<SkSurface> surface =
      context
          ? SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, image_info)
          : SkSurface::MakeRaster(image_info);

  if (!surface) {
    return nullptr;
  }

  SkCanvas* canvas = surface->getCanvas();
  canvas->clear(SK_ColorTRANSPARENT);
  canvas->translate(-dest_rect.left(), -dest_rect.top());
  canvas->concat(ctm);
  draw_function(canvas);

  if (checkerboard) {
    DrawCheckerboard(canvas, logical_rect);
  }

  return std::make_unique<RasterCacheResult>(surface->makeImageSnapshot(),
                                             logical_rect, type);
}

bool RasterCache::Prepare(const CacheableItem* cacheable_item,
                          PaintContext* paint_context) const {
  auto key = cacheable_item->GetKey();
  if (!key.has_value()) {
    return false;
  }
  Entry& entry = cache_[key.value()];
  entry.access_count++;
  entry.used_this_frame = true;
  if (!entry.image) {
    entry.image =
        cacheable_item->CreateRasterCache(paint_context, checkerboard_images_);
  }
  return true;
}

bool RasterCache::Touch(const CacheableItem* cacheable_item,
                        bool parent_cached) const {
  auto key = cacheable_item->GetKey();
  if (!key.has_value()) {
    return false;
  }
  return Touch(key.value(), parent_cached);
}

bool RasterCache::Draw(const CacheableItem* cacheable_item,
                       SkCanvas& canvas,
                       const SkPaint* paint) const {
  auto key = cacheable_item->GetKey(&canvas);
  if (!key.has_value()) {
    return false;
  }
  return Draw(key.value(), canvas, paint);
}

bool RasterCache::Touch(const RasterCacheKey& cache_key,
                        bool parent_had_cached) const {
  auto it = cache_.find(cache_key);
  if (it != cache_.end()) {
    if (parent_had_cached) {
      // current entry has beyond can live frame, try to remove it
      if (--it->second.survive_frame_count <= 0) {
        it->second.used_this_frame = false;
      }
    }
    it->second.used_this_frame = true;
    it->second.access_count++;
    return true;
  }
  return false;
}

RasterCache::Entry& RasterCache::EntryForKey(RasterCacheKey key) const {
  Entry& entry = cache_[key];
  return entry;
}

bool RasterCache::Draw(const RasterCacheKey& cache_key,
                       SkCanvas& canvas,
                       const SkPaint* paint) const {
  auto it = cache_.find(cache_key);
  if (it == cache_.end()) {
    return false;
  }

  Entry& entry = it->second;
  entry.access_count++;
  entry.used_this_frame = true;

  if (entry.image) {
    entry.image->draw(canvas, paint);
    return true;
  }

  return false;
}

void RasterCache::PrepareNewFrame() {
  picture_cached_this_frame_ = 0;
  display_list_cached_this_frame_ = 0;
}

void RasterCache::SweepOneCacheAfterFrame(RasterCacheKey::Map<Entry>& cache,
                                          RasterCacheMetrics& picture_metrics,
                                          RasterCacheMetrics& layer_metrics) {
  std::vector<RasterCacheKey::Map<Entry>::iterator> dead;

  for (auto it = cache.begin(); it != cache.end(); ++it) {
    Entry& entry = it->second;

    if (!entry.used_this_frame) {
      dead.push_back(it);
    } else if (entry.image) {
      RasterCacheKeyKind kind = it->first.kind();
      switch (kind) {
        case RasterCacheKeyKind::kPictureMetrics:
          picture_metrics.in_use_count++;
          picture_metrics.in_use_bytes += entry.image->image_bytes();
          break;
        case RasterCacheKeyKind::kLayerMetrics:
          layer_metrics.in_use_count++;
          layer_metrics.in_use_bytes += entry.image->image_bytes();
          break;
      }
    }
    entry.used_this_frame = false;
  }

  for (auto it : dead) {
    if (it->second.image) {
      RasterCacheKeyKind kind = it->first.kind();
      switch (kind) {
        case RasterCacheKeyKind::kPictureMetrics:
          picture_metrics.eviction_count++;
          picture_metrics.eviction_bytes += it->second.image->image_bytes();
          break;
        case RasterCacheKeyKind::kLayerMetrics:
          layer_metrics.eviction_count++;
          layer_metrics.eviction_bytes += it->second.image->image_bytes();
          break;
      }
    }
    cache.erase(it);
  }
}

void RasterCache::CleanupAfterFrame() {
  picture_metrics_ = {};
  layer_metrics_ = {};
  SweepOneCacheAfterFrame(cache_, picture_metrics_, layer_metrics_);
  TraceStatsToTimeline();
}

void RasterCache::Clear() {
  cache_.clear();
  picture_metrics_ = {};
  layer_metrics_ = {};
}

size_t RasterCache::GetCachedEntriesCount() const {
  return cache_.size();
}

size_t RasterCache::GetLayerCachedEntriesCount() const {
  size_t layer_cached_entries_count = 0;
  for (const auto& item : cache_) {
    if (item.first.kind() == RasterCacheKeyKind::kLayerMetrics) {
      layer_cached_entries_count++;
    }
  }
  return layer_cached_entries_count;
}

size_t RasterCache::GetPictureCachedEntriesCount() const {
  size_t picture_cached_entries_count = 0;
  for (const auto& item : cache_) {
    if (item.first.kind() == RasterCacheKeyKind::kPictureMetrics) {
      picture_cached_entries_count++;
    }
  }
  return picture_cached_entries_count;
}

void RasterCache::SetCheckboardCacheImages(bool checkerboard) {
  if (checkerboard_images_ == checkerboard) {
    return;
  }

  checkerboard_images_ = checkerboard;

  // Clear all existing entries so previously rasterized items (with or without
  // a checkerboard) will be refreshed in subsequent passes.
  Clear();
}

void RasterCache::TraceStatsToTimeline() const {
#if !FLUTTER_RELEASE
  FML_TRACE_COUNTER(
      "flutter",                                                           //
      "RasterCache", reinterpret_cast<int64_t>(this),                      //
      "LayerCount", layer_metrics_.total_count(),                          //
      "LayerMBytes", layer_metrics_.total_bytes() / kMegaByteSizeInBytes,  //
      "PictureCount", picture_metrics_.total_count(),                      //
      "PictureMBytes", picture_metrics_.total_bytes() / kMegaByteSizeInBytes);

#endif  // !FLUTTER_RELEASE
}

size_t RasterCache::EstimateLayerCacheByteSize() const {
  size_t layer_cache_bytes = 0;
  for (const auto& item : cache_) {
    if (item.first.kind() == RasterCacheKeyKind::kLayerMetrics &&
        item.second.image) {
      layer_cache_bytes += item.second.image->image_bytes();
    }
  }
  return layer_cache_bytes;
}

size_t RasterCache::EstimatePictureCacheByteSize() const {
  size_t picture_cache_bytes = 0;
  for (const auto& item : cache_) {
    if (item.first.kind() == RasterCacheKeyKind::kPictureMetrics &&
        item.second.image) {
      picture_cache_bytes += item.second.image->image_bytes();
    }
  }
  return picture_cache_bytes;
}

}  // namespace flutter
