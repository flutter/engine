// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_KEY_H_
#define FLUTTER_FLOW_RASTER_CACHE_KEY_H_

#include <unordered_map>

#include "flutter/fml/hash_combine.h"
#include "flutter/fml/logging.h"
#include "third_party/skia/include/core/SkMatrix.h"

namespace flutter {

enum class RasterCacheKeyKind { kLayer, kPicture, kDisplayList };

class RasterCacheKey {
 public:
  RasterCacheKey(uint64_t id, RasterCacheKeyKind kind, const SkMatrix& ctm)
      : id_(id), kind_(kind), matrix_(ctm) {
    matrix_[SkMatrix::kMTransX] = 0;
    matrix_[SkMatrix::kMTransY] = 0;
  }

  uint64_t id() const { return id_; }
  RasterCacheKeyKind kind() const { return kind_; }
  const SkMatrix& matrix() const { return matrix_; }

  struct Hash {
    std::size_t operator()(RasterCacheKey const& key) const {
      return fml::HashCombine(key.id_, key.kind_);
    }
  };

  struct Equal {
    constexpr bool operator()(const RasterCacheKey& lhs,
                              const RasterCacheKey& rhs) const {
      return lhs.id_ == rhs.id_ && lhs.kind_ == rhs.kind_ &&
             lhs.matrix_ == rhs.matrix_;
    }
  };

  template <class Value>
  using Map = std::unordered_map<RasterCacheKey, Value, Hash, Equal>;

 private:
  uint64_t id_;

  RasterCacheKeyKind kind_;

  // ctm where only fractional (0-1) translations are preserved:
  //   matrix_ = ctm;
  //   matrix_[SkMatrix::kMTransX] = SkScalarFraction(ctm.getTranslateX());
  //   matrix_[SkMatrix::kMTransY] = SkScalarFraction(ctm.getTranslateY());
  SkMatrix matrix_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHE_KEY_H_
