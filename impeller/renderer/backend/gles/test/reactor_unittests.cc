// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include "flutter/testing/testing.h"  // IWYU pragma: keep
#include "gtest/gtest.h"
#include "impeller/renderer/backend/gles/handle_gles.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"
#include "impeller/renderer/backend/gles/reactor_gles.h"
#include "impeller/renderer/backend/gles/test/mock_gles.h"

namespace impeller {
namespace testing {

class TestWorker : public ReactorGLES::Worker {
 public:
  bool CanReactorReactOnCurrentThreadNow(
      const ReactorGLES& reactor) const override {
    return true;
  }
};

namespace {
static int testo = 0;
}

// This test just checks that the proc table is initialized correctly.
//
// If this test doesn't pass, no test that uses the proc table will pass.
TEST(ReactorGLES, CanInitialize) {
  auto mock_gles = MockGLES::Init();
  ProcTableGLES::Resolver resolver = kMockResolverGLES;
  auto proc_table = std::make_unique<ProcTableGLES>(resolver);
  auto worker = std::make_shared<TestWorker>();
  auto reactor = std::make_shared<ReactorGLES>(std::move(proc_table));
  reactor->AddWorker(worker);

  auto handle = reactor->CreateHandle(HandleType::kTexture, 1123);
  auto added = reactor->RegisterCleanupCallback(handle, [](void* data) {
    testo++;
  }, nullptr);

  EXPECT_TRUE(added);
  EXPECT_TRUE(reactor->React());

  reactor->CollectHandle(handle);
  EXPECT_TRUE(reactor->AddOperation([](const ReactorGLES& reactor) {}));
  EXPECT_TRUE(reactor->React());
  EXPECT_EQ(testo, 1);
}

}  // namespace testing
}  // namespace impeller