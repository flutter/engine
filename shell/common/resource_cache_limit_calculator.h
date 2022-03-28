// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_VIEWPORT_SIZE_HOLDER_
#define FLUTTER_SHELL_COMMON_VIEWPORT_SIZE_HOLDER_

#include <cstdint>
#include <unordered_map>

#include "flutter/fml/macros.h"

namespace flutter {
class ResourceCacheLimitCalculator {
 public:
  ResourceCacheLimitCalculator(size_t max_bytes_threshold)
      : max_bytes_threshold_(max_bytes_threshold) {}
  ~ResourceCacheLimitCalculator() = default;
  void UpdateResourceCacheBytes(uintptr_t id, size_t resource_cache_bytes);
  void RemoveResourceCacheBytes(uintptr_t id);
  size_t GetResourceCacheBytes(uintptr_t id);
  size_t GetResourceCacheMaxBytes();
  void UpdateMaxBytesThreshold(size_t max_bytes_threshold) {
    max_bytes_threshold_ = max_bytes_threshold;
  }

 private:
  std::unordered_map<uintptr_t, size_t> map_;
  size_t max_bytes_threshold_;
  FML_DISALLOW_COPY_AND_ASSIGN(ResourceCacheLimitCalculator);
};
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_VIEWPORT_SIZE_HOLDER_
