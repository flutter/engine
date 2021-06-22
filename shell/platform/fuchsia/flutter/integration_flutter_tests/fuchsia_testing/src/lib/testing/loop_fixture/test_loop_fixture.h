// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_TESTING_LOOP_FIXTURE_TEST_LOOP_FIXTURE_H_
#define SRC_LIB_TESTING_LOOP_FIXTURE_TEST_LOOP_FIXTURE_H_

#include <gtest/gtest.h>

#include "test_loop.h"

namespace gtest {
// An extension of Test class which sets up a message loop for
// the test.
//
// Example:
//
//   class FooTest : public ::gtest::TestLoopFixture { /* ... */ };
//
//   TEST_F(FooTest, TestCase) {
//
//     // Initialize an object with the underlying loop's dispatcher.
//     Foo foo(dispatcher());
//
//     /* Call a method of |foo| that posts a delayed task. */
//
//     RunLoopFor(zx::sec(17));
//
//     /* Make assertions about the state of the test case, say about |foo|. */
//   }
class TestLoopFixture : public ::loop_fixture::TestLoop, public ::testing::Test {};

}  // namespace gtest

#endif  // SRC_LIB_TESTING_LOOP_FIXTURE_TEST_LOOP_FIXTURE_H_
