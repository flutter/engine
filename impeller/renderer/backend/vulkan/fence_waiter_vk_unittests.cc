// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"  // IWYU pragma: keep
#include "fml/macros.h"
#include "impeller/renderer/backend/vulkan/fence_waiter_vk.h"
#include "impeller/renderer/backend/vulkan/test/mock_vulkan.h"
#include "impeller/renderer/backend/vulkan/vk.h"  // IWYU pragma: keep

namespace impeller {
namespace testing {

// FIXME: Move this to mock_vulkan.cc assuming it's the right abstraction.
namespace {

static int next_fence_id = 1;

class FakeUniqueFence final {
 public:
  static vk::UniqueFence* Create() {
    auto fence = new FakeUniqueFence();
    return reinterpret_cast<vk::UniqueFence*>(fence);
  }

  FakeUniqueFence() : id_(next_fence_id++) {}

  int id() const { return id_; }

 private:
  int id_;

  FML_DISALLOW_COPY_AND_ASSIGN(FakeUniqueFence);
};

}  // namespace

TEST(FenceWaiterVKTest, RunsCallbackWhenFenceSignalled) {
  auto waiter = CreateMockVulkanContext()->GetFenceWaiter();
  auto fence = FakeUniqueFence::Create();

  FML_LOG(ERROR) << "Adding fence";
  auto success = waiter->AddFence(
      std::move(*fence), []() { FML_LOG(ERROR) << "Callback called"; });
  if (success) {
    FML_LOG(ERROR) << "Added fence";
  } else {
    FML_LOG(ERROR) << "Failed to add fence";
  }
}

}  // namespace testing
}  // namespace impeller
