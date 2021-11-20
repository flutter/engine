// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_TESTING_TEST_VULKAN_SURFACE_IMPL_H_
#define FLUTTER_TESTING_TEST_VULKAN_SURFACE_IMPL_H_

#include "flutter/testing/test_vulkan_context.h"

#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

namespace testing {

class TestVulkanSurface {
 public:
  TestVulkanSurface(const TestVulkanContext& context,
                    const SkISize& surface_size);

  bool IsValid() const;

  sk_sp<SkImage> GetSurfaceSnapshot() const;

 private:
  sk_sp<SkSurface> surface_;
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_TESTING_TEST_VULKAN_SURFACE_IMPL_H_
