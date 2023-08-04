
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>

#include "fml/logging.h"

namespace impeller {

/// Simple array with static capacity that does not allocate memory on heap.
template <typename T, size_t Capacity>
class LocalArray {
  static_assert(std::is_trivially_destructible_v<T>,
                "Type must be trivially destructible");

 public:
  T* data() { return data_.data(); }
  const T* data() const { return data_.data(); }

  size_t size() const { return size_; }
  size_t capacity() const { return Capacity; }

  T& operator[](size_t index) {
    FML_DCHECK(index < size_);
    return data_[index];
  }

  const T& operator[](size_t index) const {
    FML_DCHECK(index < size_);
    return data_[index];
  }

  void clear() { size_ = 0; }

  T* begin() { return data_.begin(); }
  const T* begin() const { return data_.begin(); }

  T* end() { return data_.begin() + size_; }
  const T* end() const { return data_.begin() + size_; }

  void push_back(const T& value) {
    FML_DCHECK(size_ < Capacity);
    data_[size_++] = value;
  }

 private:
  std::array<T, Capacity> data_;
  size_t size_ = 0;
};
}  // namespace impeller
