// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CONTAINER_UTILS_H_
#define BASE_CONTAINER_UTILS_H_

#include <vector>

namespace base {

// Return a C++ string given printf-like input.
template <class T, class Allocator, class Predicate>
size_t EraseIf(std::vector<T, Allocator>& container, Predicate pred) {
  auto it = std::remove_if(container.begin(), container.end(), pred);
  size_t removed = std::distance(it, container.end());
  container.erase(it, container.end());
  return removed;
}

}  // namespace base

#endif  // BASE_CONTAINER_UTILS_H_
