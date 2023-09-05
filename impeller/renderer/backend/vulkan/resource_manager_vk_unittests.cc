// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/types.h>
#include "flutter/testing/testing.h"
#include "impeller/renderer/backend/vulkan/resource_manager_vk.h"

namespace impeller {
namespace testing {

// While expected to be a singleton per context, the class does not enforce it.
TEST(ResourceManagerVKTest, CreateCreatesANewInstance) {
  auto const a = ResourceManagerVK::Create();
  auto const b = ResourceManagerVK::Create();
  EXPECT_NE(a, b);
}

}  // namespace testing
}  // namespace impeller
