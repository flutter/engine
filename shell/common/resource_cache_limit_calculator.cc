// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/resource_cache_limit_calculator.h"

namespace flutter {

void ResourceCacheLimitCalculator::UpdateResourceCacheBytes(
    void* key,
    size_t resource_cache_bytes) {
  map_[key] = resource_cache_bytes;
}

void ResourceCacheLimitCalculator::RemoveResourceCacheBytes(void* key) {
  auto it = map_.find(key);
  if (it == map_.end()) {
    return;
  }
  map_.erase(it);
}

size_t ResourceCacheLimitCalculator::GetResourceCacheBytes(void* key) {
  auto it = map_.find(key);
  if (it == map_.end()) {
    return 0;
  }
  return it->second;
}

size_t ResourceCacheLimitCalculator::GetResourceCacheMaxBytes() {
  size_t max_bytes = 0;
  size_t max_bytes_threshold = max_bytes_threshold_ > 0
                                   ? max_bytes_threshold_
                                   : std::numeric_limits<size_t>::max();
  for (auto it = map_.begin(); it != map_.end(); ++it) {
    max_bytes += it->second;
    if (max_bytes >= max_bytes_threshold) {
      return max_bytes_threshold;
    }
  }
  return max_bytes;
}

}  // namespace flutter
