// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLOW_SEMAPHORE_H_
#define FLOW_SEMAPHORE_H_

#include <memory>

#include "lib/ftl/macros.h"
#include "lib/ftl/time/time_delta.h"
#include "lib/ftl/compiler_specific.h"

namespace flow {

class PlatformSemaphore;

class Semaphore {
 public:
  explicit Semaphore(uint32_t count);

  ~Semaphore();

  bool IsValid() const;

  FTL_WARN_UNUSED_RESULT
  bool TryWait();

  void Signal();

 private:
  std::unique_ptr<PlatformSemaphore> _impl;

  FTL_DISALLOW_COPY_AND_ASSIGN(Semaphore);
};

}  // namespace flow

#endif  // FLOW_SEMAPHORE_H_
