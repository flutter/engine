// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_MOCK_RASTER_CACHE_H_
#define TESTING_MOCK_RASTER_CACHE_H_

#include "flutter/flow/raster_cache.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkImage.h"

namespace flutter {
namespace testing {

// Mock |RasterCache|, useful for writing tests that impact the Flutter engine
// raster_cache without requiring a GPU.
//
// The |MockCanvas| stores a list of Engine Layers and Pictures that have been
// inserted into the cache that the test can later verify against the expected
// list of cached objects.
class MockRasterCache : public RasterCache {
 public:
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
               bool will_change) override;

  void Prepare(PrerollContext* context, Layer* layer, const SkMatrix& ctm) override;

  RasterCacheResult Get(const SkPicture& picture, const SkMatrix& ctm) const override;

  RasterCacheResult Get(Layer* layer, const SkMatrix& ctm) const override;

  bool WasPrepared(Layer* layer, const SkMatrix& ctm);

  void SweepAfterFrame();

  void Clear();

  int PictureCacheCount() { return picture_cache_.size(); }
  int LayerCacheCount() { return layer_cache_.size(); }

  void SetCheckboardCacheImages(bool checkerboard);

  size_t GetCachedEntriesCount() const;

  MockRasterCache();

 protected:

 private:
  struct Entry {
    bool used_this_frame = false;
    size_t access_count = 0;
    RasterCacheResult image;
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

  mutable PictureRasterCacheKey::Map<Entry> picture_cache_;
  mutable LayerRasterCacheKey::Map<Entry> layer_cache_;
};

}  // namespace testing
}  // namespace flutter

#endif  // TESTING_MOCK_RASTER_CACHE_H_
