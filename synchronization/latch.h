// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNCHRONIZATION_LATCH_
#define SYNCHRONIZATION_LATCH_

#include <atomic>

#include "lib/ftl/macros.h"
#include "lib/ftl/synchronization/mutex.h"
#include "lib/ftl/synchronization/cond_var.h"

namespace flutter {

/// A simple countdown latch used as a thread synchronization primitive.
class Latch final {
 public:
  /// Create a latch with the specified count.
  explicit Latch(size_t count);

  ~Latch();

  /// Blocks the current thread till the count reaches zero. If the count is
  /// already zero, this opertation is a no-op.
  void Wait();

  ///  Decrement the count of the latch.
  void CountDown();

  /// Reset the count of the latch.
  void Reset();

 private:
  size_t initial_;
  std::atomic_size_t count_;
  ftl::CondVar condition_;
  ftl::Mutex condition_mutex_;

  FTL_DISALLOW_COPY_AND_ASSIGN(Latch);
};

}  // namespace flutter

#endif  // SYNCHRONIZATION_LATCH_
