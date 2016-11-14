// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_H_
#define FLUTTER_FLOW_RASTER_CACHE_H_

#include <memory>
#include <unordered_map>

#include "flutter/flow/instrumentation.h"
#include "lib/ftl/macros.h"
#include "lib/ftl/memory/ref_counted.h"
#include "lib/ftl/memory/weak_ptr.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkSize.h"

namespace flow {

class RasterCacheImage : public ftl::RefCountedThreadSafe<RasterCacheImage> {
  FRIEND_MAKE_REF_COUNTED(RasterCacheImage);
  FRIEND_REF_COUNTED_THREAD_SAFE(RasterCacheImage);

 public:
  /// The cached image.
  sk_sp<SkImage> image() const { return image_; }

  /// The bounds of the cached image.
  const SkRect& bounds() const { return bounds_; }

 private:
  sk_sp<SkImage> image_;
  SkRect bounds_;

  RasterCacheImage(sk_sp<SkImage> image, SkRect bounds);

  ~RasterCacheImage();

  FTL_DISALLOW_COPY_AND_ASSIGN(RasterCacheImage);
};

class RasterCache {
 public:
  RasterCache();

  ~RasterCache();

  ftl::RefPtr<RasterCacheImage> GetPrerolledImage(GrContext* context,
                                                  SkPicture* picture,
                                                  const SkRect& picture_bounds,
                                                  const SkMatrix& ctm,
                                                  bool is_complex,
                                                  bool will_change);
  void SweepAfterFrame();

  void Clear();

  void SetCheckboardCacheImages(bool checkerboard);

 private:
  struct Entry {
    Entry();
    ~Entry();

    bool used_this_frame = false;
    int access_count = 0;
    SkISize physical_size;
    ftl::RefPtr<RasterCacheImage> image;
  };

  using Cache = std::unordered_map<uint32_t, Entry>;

  Cache cache_;
  bool checkerboard_images_;
  ftl::WeakPtrFactory<RasterCache> weak_factory_;

  FTL_DISALLOW_COPY_AND_ASSIGN(RasterCache);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_RASTER_CACHE_H_
