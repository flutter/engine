
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"

#include "flutter/fml/thread_pool.h"

int Add(int a, int b) {
  return a + b;
}

TEST(ThreadPool, CanStartAndEnd) {
  fml::ThreadPool pool(3);
  auto future = pool.enqueue(Add, 1, 2);
  ASSERT_TRUE(future.has_value());
  auto res = future.value().get();
  ASSERT_EQ(res, 3);
}
