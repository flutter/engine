// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_TESTING_LOOP_FIXTURE_REAL_LOOP_FIXTURE_H_
#define SRC_LIB_TESTING_LOOP_FIXTURE_REAL_LOOP_FIXTURE_H_

#include <gtest/gtest.h>

#include "real_loop.h"

namespace gtest {
// An extension of Test class which sets up a message loop,
// async::Loop, for the test. This fixture is meant to be used for
// multi-process tests.
//
// This allows, for example, a test to conveniently excercise FIDL, as FIDL
// bindings post waits to the thread-local dispatcher.
//
// Example:
//
//   #include "foo.fidl.h"
//
//   class TestFoo : public Foo {
//    public:
//      explicit TestFoo(InterfaceRequest<Foo> request)
//          : binding_(this, std::move(request) {}
//
//        // Foo implementation here.
//
//    private:
//     Binding<Foo> binding_;
//   };
//
//   // Creates a fixture that creates a message loop on this thread.
//   class TestBar : public ::gtest::RealLoopFixture { /* ... */ };
//
//   TEST_F(TestBar, TestCase) {
//     // Do all FIDL-y stuff here and asynchronously quit.
//
//     RunLoop();
//
//     // Check results from FIDL-y stuff here.
//   }
class RealLoopFixture : public ::loop_fixture::RealLoop, public ::testing::Test {};

}  // namespace gtest

#endif  // SRC_LIB_TESTING_LOOP_FIXTURE_REAL_LOOP_FIXTURE_H_
