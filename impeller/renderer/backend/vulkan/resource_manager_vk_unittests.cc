// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/types.h>
#include <functional>
#include <memory>
#include <utility>
#include "gtest/gtest.h"
#include "impeller/renderer/backend/vulkan/resource_manager_vk.h"

namespace impeller {
namespace testing {

// While expected to be a singleton per context, the class does not enforce it.
TEST(ResourceManagerVKTest, CreatesANewInstance) {
  auto const a = ResourceManagerVK::Create();
  auto const b = ResourceManagerVK::Create();
  EXPECT_NE(a, b);
}

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

TEST(ResourceManagerVKTest, ReclaimMovesAResourceAndDestroysIt) {
  auto const manager = ResourceManagerVK::Create();

  auto dead = false;
  auto rattle = DeathRattle([&dead]() { dead = true; });

  // Not killed immediately.
  EXPECT_FALSE(dead);

  {
    auto resource = UniqueResourceVKT<DeathRattle>(manager, std::move(rattle));

    // Not killed on moving.
    EXPECT_FALSE(dead);
  }

  // Not killed synchronously.
  EXPECT_FALSE(dead);

  // A background thread reclaims; give it a chance to finish killing.
  while (!dead) {
    std::this_thread::yield();
  }

  // The resource should be dead now.
  EXPECT_TRUE(dead);
}

}  // namespace testing
}  // namespace impeller
