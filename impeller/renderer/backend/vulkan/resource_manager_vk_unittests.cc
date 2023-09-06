// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/types.h>
#include "flutter/testing/testing.h"
#include "impeller/renderer/backend/vulkan/resource_manager_vk.h"

namespace impeller {
namespace testing {

// In a real app, this might be a texture or another resource buffer.
struct FakeBuffer {
  int id;
};

// While expected to be a singleton per context, the class does not enforce it.
TEST(ResourceManagerVKTest, CreateCreatesANewInstance) {
  auto const a = ResourceManagerVK::Create();
  auto const b = ResourceManagerVK::Create();
  EXPECT_NE(a, b);
}

TEST(ResourceManagerVKTest, ReclaimResetsTheUnderlyingResource) {
  auto const manager = ResourceManagerVK::Create();

  UniqueResourceVKT<FakeBuffer> resource(manager, FakeBuffer{1});
  EXPECT_EQ(resource->id, 1);

  resource.Reset();
  EXPECT_EQ(resource->id, 0);
}

TEST(ResourceManagerVKTest, CallingResetTwiceHasNoEffect) {
  auto const manager = ResourceManagerVK::Create();

  UniqueResourceVKT<FakeBuffer> resource(manager, FakeBuffer{1});
  EXPECT_EQ(resource->id, 1);

  resource.Reset();
  resource.Reset();
  EXPECT_EQ(resource->id, 0);
}

TEST(ResourceManagerVKTest, ResetWaitsForLock) {
  auto const manager = ResourceManagerVK::Create();

  // Create a resource and lock it by holding a reference to it.
  UniqueResourceVKT<FakeBuffer> resource(manager, FakeBuffer{1});
  auto const& resource_ref = resource;

  // Reset the resource in a separate thread.
  std::thread reset_thread([&resource]() { resource.Reset(); });

  // Wait for the thread to start.
  while (resource->id != 1) {
    std::this_thread::yield();
  }

  // The resource should not have been reset yet.
  EXPECT_EQ(resource->id, 1);

  // Wait for the thread to finish.
  reset_thread.join();

  // The resource should have been reset.
  EXPECT_EQ(resource->id, 0);
}

TEST(ResourceManagerVKTest, SwapReplacesTheUnderlyingResource) {
  auto const manager = ResourceManagerVK::Create();

  UniqueResourceVKT<FakeBuffer> resource(manager, FakeBuffer{1});
  EXPECT_EQ(resource->id, 1);

  resource.Swap(FakeBuffer{2});
  EXPECT_EQ(resource->id, 2);
}

// A class that, upon destruction, invokes a callback.
class Scoped {
 public:
  explicit Scoped(std::function<void()> callback)
      : callback_(std::move(callback)) {}
  ~Scoped() { callback_(); }

 private:
  std::function<void()> callback_;
};

TEST(ResourceManagerVKTest, ResourceFallingOutOfScopeResetsTheResource) {
  auto const manager = ResourceManagerVK::Create();
  auto was_destroyed = false;

  {
    UniqueResourceVKT<Scoped> resource(
        manager, Scoped([&was_destroyed]() { was_destroyed = true; }));
    EXPECT_FALSE(was_destroyed);
  }

  EXPECT_TRUE(was_destroyed);
}

}  // namespace testing
}  // namespace impeller
