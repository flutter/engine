// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "test_loop.h"

namespace loop_fixture {

TestLoop::TestLoop() = default;

TestLoop::~TestLoop() = default;

void TestLoop::RunLoopRepeatedlyFor(zx::duration increment) {
  while (RunLoopFor(increment)) {
  }
}

}  // namespace loop_fixture
