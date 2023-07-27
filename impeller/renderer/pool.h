// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

namespace impeller {

template <typename T>
class Pool {
 public:
  std::shared_ptr<T> Grab() {
    if (pool_.empty()) {
      return T::Create();
    } else {
      std::shared_ptr<T> result = pool_.back();
      pool_.pop_back();
      return result;
    }
  }

  void Recycle(std::shared_ptr<T> object) {
    object->Reset();
    pool_.emplace_back(std::move(object));
  }

 private:
  std::vector<std::shared_ptr<T>> pool_;
};

}  // namespace impeller
