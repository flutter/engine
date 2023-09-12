// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"  // IWYU pragma: keep.
#include "fml/synchronization/waitable_event.h"
#include "impeller/renderer/backend/vulkan/resource_manager_vk.h"
#include "impeller/renderer/backend/vulkan/test/mock_vulkan.h"

namespace impeller {
namespace testing {

TEST(CommandPoolRecyclerVKTest, GetsACommandPoolPerThread) {
  auto const context = CreateMockVulkanContext();

  // Record the memory location of each pointer to a command pool.
  int* pool1 = nullptr;
  int* pool2 = nullptr;

  // Create a command pool in two threads and record the memory location.
  std::thread thread1([&]() {
    auto const pool = context->GetCommandPoolRecycler()->Get();
    pool1 = reinterpret_cast<int*>(pool.get());
  });

  std::thread thread2([&]() {
    auto const pool = context->GetCommandPoolRecycler()->Get();
    pool2 = reinterpret_cast<int*>(pool.get());
  });

  thread1.join();
  thread2.join();

  // The two command pools should be different.
  EXPECT_NE(pool1, pool2);

  context->Shutdown();
}

TEST(CommandPoolRecyclerVKTest, GetsTheSameCommandPoolOnSameThread) {
  auto const context = CreateMockVulkanContext();

  auto const pool1 = context->GetCommandPoolRecycler()->Get();
  auto const pool2 = context->GetCommandPoolRecycler()->Get();

  // The two command pools should be the same.
  EXPECT_EQ(pool1.get(), pool2.get());

  context->Shutdown();
}

namespace {

// Invokes the provided callback when the destructor is called.
//
// Can be moved, but not copied.
class DeathRattle final {
 public:
  explicit DeathRattle(std::function<void()> callback)
      : callback_(std::move(callback)) {}

  DeathRattle(DeathRattle&&) = default;
  DeathRattle& operator=(DeathRattle&&) = default;

  ~DeathRattle() { callback_(); }

 private:
  std::function<void()> callback_;
};

}  // namespace

TEST(CommandPoolRecyclerVKTest, ReclaimMakesCommandPoolAvailable) {
  auto const context = CreateMockVulkanContext();

  {
    // Fetch a pool (which will be created).
    auto const recycler = context->GetCommandPoolRecycler();
    auto const pool = recycler->Get();

    // This normally is called at the end of a frame.
    recycler->Dispose();
  }

  // Add something to the resource manager and have it notify us when it's
  // destroyed. That should give us a non-flaky signal that the pool has been
  // reclaimed as well.
  auto waiter = fml::AutoResetWaitableEvent();
  auto rattle = DeathRattle([&waiter]() { waiter.Signal(); });
  {
    UniqueResourceVKT<DeathRattle> resource(context->GetResourceManager(),
                                            std::move(rattle));
  }
  waiter.Wait();

  // On another thread explicitly, request a new pool.
  std::thread thread([&]() {
    auto const pool = context->GetCommandPoolRecycler()->Get();
    EXPECT_NE(pool.get(), nullptr);
  });

  thread.join();

  // Now check that we only ever created one pool.
  auto const called = GetMockVulkanFunctions(context->GetDevice());
  EXPECT_EQ(std::count(called->begin(), called->end(), "vkCreateCommandPool"),
            1u);

  context->Shutdown();
}

}  // namespace testing
}  // namespace impeller
