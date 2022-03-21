// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_VIEWPORT_SIZE_HOLDER_
#define FLUTTER_SHELL_COMMON_VIEWPORT_SIZE_HOLDER_

#include <cstdint>
#include <unordered_map>

#include "flutter/fml/macros.h"

namespace flutter {
struct ViewportSize {
  double width = 0.0;
  double height = 0.0;
};

static inline bool operator==(const ViewportSize& a, const ViewportSize& b) {
  return a.width == b.width && a.height == b.height;
}

static inline bool operator!=(const ViewportSize& a, const ViewportSize& b) {
  return !(a == b);
}

class ViewportSizeHolder {
 public:
  ViewportSizeHolder() = default;

  ~ViewportSizeHolder() = default;

  void UpdateViewportSize(uintptr_t id, ViewportSize size);

  void RemoveViewportSize(uintptr_t id);

  ViewportSize GetViewportSize(uintptr_t id);

  ViewportSize GetMaxViewportSize() const;

 private:
  std::unordered_map<uintptr_t, ViewportSize> map_;

  FML_DISALLOW_COPY_AND_ASSIGN(ViewportSizeHolder);
};
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_VIEWPORT_SIZE_HOLDER_
