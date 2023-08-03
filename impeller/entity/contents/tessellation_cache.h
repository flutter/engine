// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/logging.h"
#include "impeller/geometry/path.h"
#include "impeller/tessellator/tessellator.h"

namespace impeller {

class TessellationCache {
 public:
  Path::Polyline GetOrCreatePolyline(const impeller::Path& path, Scalar scale);

  Tessellator::Result Tessellate(const Tessellator& tesselator,
                                 FillType fill_type,
                                 const Path::Polyline& polyline,
                                 const Tessellator::BuilderCallback& callback);

  void FinishFrame() {
    int cache_hits, cache_misses;
    polyline_cache_.FinishFrame(cache_hits, cache_misses);
    FML_LOG(INFO) << "PolylineCache: " << cache_hits << " hits, "
                  << cache_misses << " misses";

    tessellator_cache_.FinishFrame(cache_hits, cache_misses);
    FML_LOG(INFO) << "TessellatorCache: " << cache_hits << " hits, "
                  << cache_misses << " misses";
  }

 private:
  struct PolylineKey {
    std::uint32_t generation_id;
    Scalar scale;

    bool operator==(const PolylineKey& other) const {
      return generation_id == other.generation_id && scale == other.scale;
    }
  };

  struct PolylineKeyHash {
    size_t operator()(const PolylineKey& key) const {
      return key.generation_id + 31 * std::hash<Scalar>()(key.scale);
    }
  };

  struct TessellatorKey {
    std::uint32_t generation_id;
    FillType fill_type;

    bool operator==(const TessellatorKey& other) const {
      return generation_id == other.generation_id &&
             fill_type == other.fill_type;
    }
  };

  struct TessellatorKeyHash {
    size_t operator()(const TessellatorKey& key) const {
      return key.generation_id + 31 * std::hash<FillType>()(key.fill_type);
    }
  };

  struct TessellatorData {
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
  };

  template <typename K, typename V, typename Hash, typename Compare>
  class FrameAwareCache {
   public:
    std::optional<V> Get(const K& key) {
      auto it = used_.find(key);
      if (it != used_.end()) {
        ++cache_hit_count_;
        return it->second;
      }
      it = maybe_remove_.find(key);
      if (it != maybe_remove_.end()) {
        ++cache_hit_count_;
        // promote maybe_remove_ to persistent_
        auto insert_it = used_.emplace(it->first, std::move(it->second));
        maybe_remove_.erase(it);
        return insert_it.first->second;
      }
      ++cache_miss_count_;
      return std::nullopt;
    }

    void Set(const K key, V value) { used_.emplace(key, std::move(value)); }

    /// Returns number of cache misses for this frame.
    void FinishFrame(int& cache_hits, int& cache_misses) {
      maybe_remove_ = std::move(used_);
      used_.clear();
      cache_hits = cache_hit_count_;
      cache_misses = cache_miss_count_;
      cache_hit_count_ = 0;
      cache_miss_count_ = 0;
    }

   private:
    // Items used during this frame.
    std::unordered_map<K, V, Hash, Compare> used_;

    // Items that will be removed at the end of the frame unless they are
    // accessed.
    std::unordered_map<K, V, Hash, Compare> maybe_remove_;

    int cache_miss_count_ = 0;
    int cache_hit_count_ = 0;
  };

  FrameAwareCache<PolylineKey,
                  Path::Polyline,
                  PolylineKeyHash,
                  std::equal_to<PolylineKey>>
      polyline_cache_;

  FrameAwareCache<TessellatorKey,
                  TessellatorData,
                  TessellatorKeyHash,
                  std::equal_to<TessellatorKey>>
      tessellator_cache_;
};

}  // namespace impeller