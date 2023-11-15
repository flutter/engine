// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/core/formats.h"
#include "impeller/core/texture_descriptor.h"
#include "impeller/renderer/backend/metal/formats_mtl.h"
#include "impeller/renderer/backend/metal/lazy_drawable_holder.h"
#include "impeller/renderer/backend/metal/texture_mtl.h"
#include "impeller/renderer/capabilities.h"

#include <QuartzCore/CAMetalLayer.h>
#include <thread>

#include "gtest/gtest.h"

namespace impeller {
namespace testing {

TEST(TextureMTL, CreateFromDrawable) {
  auto device = MTLCreateSystemDefaultDevice();
  auto layer = [[CAMetalLayer alloc] init];
  layer.device = device;
  layer.drawableSize = CGSize{100, 100};
  layer.pixelFormat = ToMTLPixelFormat(PixelFormat::kB8G8R8A8UNormInt);

  TextureDescriptor desc;
  desc.size = {100, 100};
  desc.format = PixelFormat::kB8G8R8A8UNormInt;

  auto drawble_holder = std::make_shared<LazyDrawableHolder>(layer);
  auto drawable_texture = std::make_shared<TextureMTL>(
      desc,
      [&drawble_holder]() { return drawble_holder->AcquireNextDrawable(); },
      /*wrapped=*/false, /*drawable=*/true);

  ASSERT_TRUE(drawable_texture->IsValid());
  EXPECT_TRUE(drawable_texture->IsDrawable());

  // Spawn a thread and acquire the drawable in the thread.
  auto thread = std::thread([&drawable_texture]() {
    // Force the drawable to be acquired.
    drawable_texture->GetMTLTexture();
  });
  thread.join();
  // Block until drawable is acquired.
  EXPECT_TRUE(drawble_holder->GetDrawable() != nil);
  // Drawable is cached.
  EXPECT_TRUE(drawable_texture->GetMTLTexture() != nil);
}

}  // namespace testing
}  // namespace impeller