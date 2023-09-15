// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fml/synchronization/waitable_event.h"
#include "fml/time/time_delta.h"
#include "gtest/gtest.h"  // IWYU pragma: keep
#include "impeller/renderer/backend/vulkan/fence_waiter_vk.h"  // IWYU pragma: keep
#include "impeller/renderer/backend/vulkan/test/mock_vulkan.h"

namespace impeller {
namespace testing {

TEST(FenceWaiterVKTest, IgnoresNullFence) {
  auto const context = MockVulkanContextBuilder().Build();
  auto const waiter = context->GetFenceWaiter();
  EXPECT_FALSE(waiter->AddFence(vk::UniqueFence(), []() {}));
}

TEST(FenceWaiterVKTest, IgnoresNullCallback) {
  auto const context = MockVulkanContextBuilder().Build();
  auto const device = context->GetDevice();
  auto const waiter = context->GetFenceWaiter();

  auto fence = device.createFenceUnique({}).value;
  EXPECT_FALSE(waiter->AddFence(std::move(fence), nullptr));
}

TEST(FenceWaiterVKTest, ExecutesFenceCallback) {
  auto const context = MockVulkanContextBuilder().Build();
  auto const device = context->GetDevice();
  auto const waiter = context->GetFenceWaiter();

  auto signal = fml::ManualResetWaitableEvent();
  auto fence = device.createFenceUnique({}).value;
  waiter->AddFence(std::move(fence), [&signal]() { signal.Signal(); });

  signal.Wait();
}

TEST(FenceWaiterVKTest, ExecutesFenceCallbackX2) {
  auto const context = MockVulkanContextBuilder().Build();
  auto const device = context->GetDevice();
  auto const waiter = context->GetFenceWaiter();

  auto signal = fml::ManualResetWaitableEvent();
  auto fence = device.createFenceUnique({}).value;
  waiter->AddFence(std::move(fence), [&signal]() { signal.Signal(); });

  auto signal2 = fml::ManualResetWaitableEvent();
  auto fence2 = device.createFenceUnique({}).value;
  waiter->AddFence(std::move(fence2), [&signal2]() { signal2.Signal(); });

  signal.Wait();
  signal2.Wait();
}

TEST(FenceWaiterVKTest, ExecutesNewFenceThenOldFence) {
  auto const context = MockVulkanContextBuilder().Build();
  auto const device = context->GetDevice();
  auto const waiter = context->GetFenceWaiter();

  auto signal = fml::ManualResetWaitableEvent();
  auto fence = device.createFenceUnique({}).value;
  MockFence::SetStatus(fence, vk::Result::eNotReady);
  auto raw_fence = MockFence::GetRawPointer(fence);
  waiter->AddFence(std::move(fence), [&signal]() { signal.Signal(); });

  // The easiest way to verify that the callback was _not_ called is to wait
  // for a timeout, but that could introduce flakiness. Instead, we'll add a
  // second fence that will signal immediately, and wait for that one instead.
  {
    auto signal2 = fml::ManualResetWaitableEvent();
    auto fence2 = device.createFenceUnique({}).value;
    MockFence::SetStatus(fence2, vk::Result::eSuccess);
    waiter->AddFence(std::move(fence2), [&signal2]() { signal2.Signal(); });
    signal2.Wait();
  }

  // Now, we'll signal the first fence, and wait for the callback to be called.
  raw_fence->SetStatus(vk::Result::eSuccess);

  // Now, we'll signal the first fence, and wait for the callback to be called.
  signal.Wait();
}

TEST(FenceWaiterVKTest, StillDestroysFenceIfTerminating) {
  auto signal = fml::ManualResetWaitableEvent();

  {
    auto const context = MockVulkanContextBuilder().Build();
    auto const device = context->GetDevice();
    auto const waiter = context->GetFenceWaiter();
    waiter->Terminate();

    auto fence = device.createFenceUnique({}).value;
    waiter->AddFence(std::move(fence), [&signal]() { signal.Signal(); });
  }

  // Ensure the fence still triggers.
  signal.Wait();
}

TEST(FenceWaiterVKTest, InProgressFencesStillWaitIfTerminated) {
  MockFence* raw_fence = nullptr;

  {
    auto const context = MockVulkanContextBuilder().Build();
    auto const device = context->GetDevice();
    auto const waiter = context->GetFenceWaiter();

    // Add a fence that isn't signalled yet.
    auto fence = device.createFenceUnique({}).value;

    // Even if the fence is eSuccess, it's not guaranteed to be called in time.
    MockFence::SetStatus(fence, vk::Result::eNotReady);
    raw_fence = MockFence::GetRawPointer(fence);
    waiter->AddFence(std::move(fence), []() {
      // Intentionally empty, imagine this is holding on to a reference.
    });

    // Spawn a thread to signal the fence.
    std::thread([&]() {
      // Wait 1 second.
      std::this_thread::sleep_for(std::chrono::seconds(1));

      // Signal the fence.
      raw_fence->SetStatus(vk::Result::eSuccess);
    }).detach();

    // Terminate the waiter by letting it drop out of scope.
  }

  // This will hang if the fence was not signalled.
}

}  // namespace testing
}  // namespace impeller
