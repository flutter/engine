// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <mutex>

namespace impeller {

template <typename T>
class AllocationPool {
 public:
  Pool(uint32_t limit_bytes) : limit_bytes_(limit_bytes), size_(0) {}

  std::shared_ptr<T> Grab() {
    std::scoped_lock lock(mutex_);
    if (pool_.empty()) {
      return T::Create();
    } else {
      std::shared_ptr<T> result = pool_.back();
      pool_.pop_back();
      size_ += result->GetReservedLength();
      return result;
    }
  }

  void Recycle(std::shared_ptr<T> object) {
    std::scoped_lock lock(mutex_);
    size_t object_size = object->GetReservedLength();
    if (size_ + object_size <= limit_bytes_ &&
        object_size < (limit_bytes_ / 2)) {
      object->Reset();
      size_ += object_size;
      pool_.emplace_back(std::move(object));
    }
  }

 private:
  std::vector<std::shared_ptr<T>> pool_;
  const uint32_t limit_bytes_;
  uint32_t size_;
  // Note: This would perform better as a lockless ring buffer.
  std::mutex mutex_;
};

}  // namespace impeller
