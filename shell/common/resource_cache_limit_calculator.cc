// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/resource_cache_limit_calculator.h"

namespace flutter {

size_t ResourceCacheLimitCalculator::GetResourceCacheMaxBytes() {
  size_t max_bytes = 0;
  size_t max_bytes_threshold = max_bytes_threshold_ > 0
                                   ? max_bytes_threshold_
                                   : std::numeric_limits<size_t>::max();
  auto it = items_.begin();
  while (it != items_.end()) {
    fml::WeakPtr<ResourceCacheLimitItem> item = *it;
    if (item) {
      max_bytes += item->GetResourceCacheLimit();
      ++it;
    } else {
      it = items_.erase(it);
    }
  }
  return std::min(max_bytes, max_bytes_threshold);
}

}  // namespace flutter
