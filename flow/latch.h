// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LATCH_H_
#define FLUTTER_FLOW_LATCH_H_

#include <atomic>

#include "lib/ftl/macros.h"
#include "lib/ftl/synchronization/cond_var.h"
#include "lib/ftl/synchronization/mutex.h"

namespace flow {

class Latch {
 public:
  // Create a latch with the specified count
  explicit Latch(size_t count);

  ~Latch();

  // Blocks the current thread till the count reaches zero. If the count is
  // already zero, this operation is a no-op
  void wait();

  // Decrement the count of the latch
  void countDown();

 private:
  std::atomic_size_t count_;
  ftl::CondVar condition_;
  ftl::Mutex condition_mutex_;

  FTL_DISALLOW_COPY_AND_ASSIGN(Latch);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_LATCH_H_
