// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/synchronization/latch.h"

namespace flutter {

Latch::Latch(size_t count) : initial_(count), count_(count) {}

Latch::~Latch() = default;

void Latch::Wait() {
  if (count_ > 0) {
    ftl::MutexLocker lock(&condition_mutex_);
    while (count_ != 0) {
      condition_.Wait(&condition_mutex_);
    }
  }
}

void Latch::CountDown() {
  if (--count_ == 0) {
    condition_.SignalAll();
  }
}

void Latch::Reset() {
  count_ = initial_;
}

}  // namespace flutter
