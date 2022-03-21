// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/viewport_size_holder.h"

namespace flutter {

void ViewportSizeHolder::UpdateViewportSize(uintptr_t id, ViewportSize size) {
  map_[id] = size;
}

void ViewportSizeHolder::RemoveViewportSize(uintptr_t id) {
  auto it = map_.find(id);
  if (it == map_.end()) {
    return;
  }
  map_.erase(it);
}

ViewportSize ViewportSizeHolder::GetViewportSize(uintptr_t id) {
  return map_[id];
}

ViewportSize ViewportSizeHolder::GetMaxViewportSize() const {
  ViewportSize max_size;
  for (auto it = map_.begin(); it != map_.end(); ++it) {
    auto& size = it->second;
    if (size.width * size.height > max_size.width * max_size.height) {
      max_size = size;
    }
  }
  return max_size;
}

}  // namespace flutter
