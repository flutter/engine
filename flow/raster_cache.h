// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_H_
#define FLUTTER_FLOW_RASTER_CACHE_H_

#include <memory>
#include <unordered_map>

#include "flutter/flow/instrumentation.h"
#include "flutter/flow/raster_cache_key.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkSize.h"

namespace flutter {

// |RasterCacheResult| contains the result of rendering a layer or a picture
// at a given transform by an instance of |RasterCacheImageDelegate| and
// provides the capability to render the result to a given |SkCanvas|.
class RasterCacheResult {
 public:
  virtual ~RasterCacheResult() = default;

  // Draw the rendered result to the |SkCanvas| using the properties of the
  // |SkPaint|.
  virtual void draw(SkCanvas& canvas, const SkPaint* paint = nullptr) const = 0;

  // Return the size of the cached result to help manage the cache memory
  // consumption.
  virtual SkISize image_dimensions() const = 0;
};

// |SkRasterCacheResult| is the typical implementation of |SkRasterCacheResult|
// created from an |SkRasterCacheImageDelegate| and storing the result as
// pixels in an |SkImage|.
class SkRasterCacheResult : public RasterCacheResult {
 public:
  SkRasterCacheResult(sk_sp<SkImage> image, const SkRect& logical_rect);

  void draw(SkCanvas& canvas, const SkPaint* paint = nullptr) const override;

  SkISize image_dimensions() const override {
    return image_ ? image_->dimensions() : SkISize::Make(0, 0);
  };

 private:
  sk_sp<SkImage> image_;
  SkRect logical_rect_;
};

struct PrerollContext;

// The |RasterCacheImageDelegate| creates and retruns a cached result of
// rendering the supplied |Layer| or |SkPicture|.
class RasterCacheImageDelegate {
 public:
  virtual std::unique_ptr<RasterCacheResult> RasterizePicture(
      SkPicture* picture,
      GrContext* context,
      const SkMatrix& ctm,
      SkColorSpace* dst_color_space,
      bool checkerboard) const = 0;

  virtual std::unique_ptr<RasterCacheResult> RasterizeLayer(
      PrerollContext* context,
      Layer* layer,
      const SkMatrix& ctm,
      bool checkerboard) const = 0;
};

// The |SkRasterCacheImageDelegate| creates and returns rendered results
// for |Layer| and |SkPicture| objects stored as |SkImage| objects.
class SkRasterCacheImageDelegate : public RasterCacheImageDelegate {
 public:
  std::unique_ptr<RasterCacheResult> RasterizePicture(
      SkPicture* picture,
      GrContext* context,
      const SkMatrix& ctm,
      SkColorSpace* dst_color_space,
      bool checkerboard) const override;

  std::unique_ptr<RasterCacheResult> RasterizeLayer(
      PrerollContext* context,
      Layer* layer,
      const SkMatrix& ctm,
      bool checkerboard) const override;

  static SkRasterCacheImageDelegate instance;
};

class RasterCache {
 public:
  // The default max number of picture raster caches to be generated per frame.
  // Generating too many caches in one frame may cause jank on that frame. This
  // limit allows us to throttle the cache and distribute the work across
  // multiple frames.
  static constexpr int kDefaultPictureCacheLimitPerFrame = 3;

  // Create a default |RasterCache| object that caches |Layer| and |SkPicture|
  // objects using a |SkRasterCacheImageDelegate|.
  //
  // @param access_threshold
  //     the number of attempts to cache a given |SkPicture| before inserting
  //     it into the cache. The defalt value of 3 will only cache a picture
  //     on the 3rd time it appears in consecutive frames.
  // @param picture_cache_limit_per_frame
  //     the maximum number of |SkPicture| objects to render into the cache
  //     per frame. The default value of 3 will rasterize at most 3 pictures
  //     in a given frame and leave the rest for caching in subsequent frames.
  //
  // @see |kDefaultPictureCacheLimitPerFrame|
  explicit RasterCache(
      size_t access_threshold = 3,
      size_t picture_cache_limit_per_frame = kDefaultPictureCacheLimitPerFrame);

  // Create a |RasterCache| object that uses a specified implementation of
  // |RasterCacheImageDelegate|.
  //
  // @see |RasterCache(size_t, size_t)|
  explicit RasterCache(
      RasterCacheImageDelegate* delegate,
      size_t access_threshold = 3,
      size_t picture_cache_limit_per_frame = kDefaultPictureCacheLimitPerFrame);

  static SkIRect GetDeviceBounds(const SkRect& rect, const SkMatrix& ctm) {
    SkRect device_rect;
    ctm.mapRect(&device_rect, rect);
    SkIRect bounds;
    device_rect.roundOut(&bounds);
    return bounds;
  }

  /**
   * @brief Snap the translation components of the matrix to integers.
   *
   * The snapping will only happen if the matrix only has scale and translation
   * transformations.
   *
   * @param ctm the current transformation matrix.
   * @return SkMatrix the snapped transformation matrix.
   */
  static SkMatrix GetIntegralTransCTM(const SkMatrix& ctm) {
    // Avoid integral snapping if the matrix has complex transformation to avoid
    // the artifact observed in https://github.com/flutter/flutter/issues/41654.
    if (!ctm.isScaleTranslate()) {
      return ctm;
    }
    SkMatrix result = ctm;
    result[SkMatrix::kMTransX] = SkScalarRoundToScalar(ctm.getTranslateX());
    result[SkMatrix::kMTransY] = SkScalarRoundToScalar(ctm.getTranslateY());
    return result;
  }

  // Return true if the cache is generated.
  //
  // We may return false and not generate the cache if
  // 1. The picture is not worth rasterizing
  // 2. The matrix is singular
  // 3. The picture is accessed too few times
  // 4. There are too many pictures to be cached in the current frame.
  //    (See also kDefaultPictureCacheLimitPerFrame.)
  bool Prepare(GrContext* context,
               SkPicture* picture,
               const SkMatrix& transformation_matrix,
               SkColorSpace* dst_color_space,
               bool is_complex,
               bool will_change);

  void Prepare(PrerollContext* context, Layer* layer, const SkMatrix& ctm);

  // Find the raster cache for the picture and draw it to the canvas.
  //
  // Return true if it's found and drawn.
  bool Draw(const SkPicture& picture, SkCanvas& canvas) const;

  // Find the raster cache for the layer and draw it to the canvas.
  //
  // Addional paint can be given to change how the raster cache is drawn (e.g.,
  // draw the raster cache with some opacity).
  //
  // Return true if the layer raster cache is found and drawn.
  bool Draw(const Layer* layer,
            SkCanvas& canvas,
            SkPaint* paint = nullptr) const;

  void SweepAfterFrame();

  void Clear();

  void SetCheckboardCacheImages(bool checkerboard);

  size_t GetCachedEntriesCount() const;

  size_t GetLayerCachedEntriesCount() const;

  size_t GetPictureCachedEntriesCount() const;

 private:
  struct Entry {
    bool used_this_frame = false;
    size_t access_count = 0;
    std::unique_ptr<RasterCacheResult> image;
  };

  template <class Cache>
  static void SweepOneCacheAfterFrame(Cache& cache) {
    std::vector<typename Cache::iterator> dead;

    for (auto it = cache.begin(); it != cache.end(); ++it) {
      Entry& entry = it->second;
      if (!entry.used_this_frame) {
        dead.push_back(it);
      }
      entry.used_this_frame = false;
    }

    for (auto it : dead) {
      cache.erase(it);
    }
  }

  const RasterCacheImageDelegate* delegate_;
  const size_t access_threshold_;
  const size_t picture_cache_limit_per_frame_;
  size_t picture_cached_this_frame_ = 0;
  mutable PictureRasterCacheKey::Map<Entry> picture_cache_;
  mutable LayerRasterCacheKey::Map<Entry> layer_cache_;
  bool checkerboard_images_;

  void TraceStatsToTimeline() const;

  FML_DISALLOW_COPY_AND_ASSIGN(RasterCache);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHE_H_
