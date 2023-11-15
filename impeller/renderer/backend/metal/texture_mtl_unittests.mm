// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/core/formats.h"
#include "impeller/core/texture_descriptor.h"
#include "impeller/renderer/backend/metal/formats_mtl.h"
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
  layer.pixelFormat = ToMTLPixelFormat(PixelFormat::kR8G8B8A8UNormInt);

  TextureDescriptor desc;
  desc.size = {100, 100};
  desc.format = PixelFormat::kR8G8B8A8UNormInt;

  auto drawable_texture = TextureMTL::WrapDrawable(desc, layer);

  ASSERT_TRUE(drawable_texture->IsValid());
  EXPECT_TRUE(drawable_texture->IsDrawableBacked());

  // Spawn a thread and acquire the drawable in the thread.
  auto thread = std::thread([&drawable_texture]() {
    // Force the drawable to be acquired.
    drawable_texture->GetMTLTexture();
  });
  thread.join();
  // Block until drawable is acquired.
  EXPECT_TRUE(drawable_texture->WaitForNextDrawable() != nil);
  // Drawable is cached.
  EXPECT_TRUE(drawable_texture->GetMTLTexture() != nil);
}

TEST(TextureMTL, CreateFromInvalidDescriptor) {
  auto device = MTLCreateSystemDefaultDevice();
  auto layer = [[CAMetalLayer alloc] init];
  layer.device = device;
  layer.drawableSize = CGSize{100, 100};
  layer.pixelFormat = ToMTLPixelFormat(PixelFormat::kR8G8B8A8UNormInt);

  TextureDescriptor desc;
  // Size is mismatched.
  desc.size = {200, 200};
  desc.format = PixelFormat::kR8G8B8A8UNormInt;

  auto drawable_texture_a = TextureMTL::WrapDrawable(desc, layer);
  EXPECT_FALSE(drawable_texture_a->IsValid());

  desc.size = {100, 100};
  // Pixel format is mismatched.
  desc.format = PixelFormat::kD24UnormS8Uint;

  auto drawable_texture_b = TextureMTL::WrapDrawable(desc, layer);
  EXPECT_FALSE(drawable_texture_b->IsValid());
}

}  // namespace testing
}  // namespace impeller