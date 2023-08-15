// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include "flutter/testing/testing.h"
#include "impeller/core/allocator.h"
#include "impeller/core/formats.h"
#include "impeller/core/texture_descriptor.h"
#include "impeller/geometry/size.h"
#include "impeller/renderer/testing/mocks.h"

namespace impeller {
namespace testing {

// class TestAllocator : public Allocator {
//  public:
//   TestAllocator() = default;

//   ~TestAllocator() = default;

//   std::vector<Allocator::TextureData> GetCachedData() {
//     return data_to_recycle_;
//   };

//   ISize GetMaxTextureSizeSupported() const override {
//     return ISize(1024, 1024);
//   };

//   std::shared_ptr<DeviceBuffer> OnCreateBuffer(
//       const DeviceBufferDescriptor& desc) override {
//     return std::make_shared<MockDeviceBuffer>(desc);
//   };

//   virtual std::shared_ptr<Texture> OnCreateTexture(
//       const TextureDescriptor& desc) override {
//     return std::make_shared<MockTexture>(desc);
//   };
// };

// TEST(AllocatorTest, TextureDescriptorCompatibility) {
//   // Size.
//   {
//     TextureDescriptor desc_a = {.size = ISize(100, 100)};
//     TextureDescriptor desc_b = {.size = ISize(100, 100)};
//     TextureDescriptor desc_c = {.size = ISize(101, 100)};

//     ASSERT_TRUE(desc_a.IsCompatibleWith(desc_b));
//     ASSERT_FALSE(desc_a.IsCompatibleWith(desc_c));
//   }
//   // Storage Mode.
//   {
//     TextureDescriptor desc_a = {.storage_mode = StorageMode::kDevicePrivate};
//     TextureDescriptor desc_b = {.storage_mode = StorageMode::kDevicePrivate};
//     TextureDescriptor desc_c = {.storage_mode = StorageMode::kHostVisible};

//     ASSERT_TRUE(desc_a.IsCompatibleWith(desc_b));
//     ASSERT_FALSE(desc_a.IsCompatibleWith(desc_c));
//   }
//   // Format.
//   {
//     TextureDescriptor desc_a = {.format = PixelFormat::kR8G8B8A8UNormInt};
//     TextureDescriptor desc_b = {.format = PixelFormat::kR8G8B8A8UNormInt};
//     TextureDescriptor desc_c = {.format = PixelFormat::kB10G10R10A10XR};

//     ASSERT_TRUE(desc_a.IsCompatibleWith(desc_b));
//     ASSERT_FALSE(desc_a.IsCompatibleWith(desc_c));
//   }
//   // Sample Count.
//   {
//     TextureDescriptor desc_a = {.sample_count = SampleCount::kCount4};
//     TextureDescriptor desc_b = {.sample_count = SampleCount::kCount4};
//     TextureDescriptor desc_c = {.sample_count = SampleCount::kCount1};

//     ASSERT_TRUE(desc_a.IsCompatibleWith(desc_b));
//     ASSERT_FALSE(desc_a.IsCompatibleWith(desc_c));
//   }
//   // Sample Count.
//   {
//     TextureDescriptor desc_a = {.type = TextureType::kTexture2DMultisample};
//     TextureDescriptor desc_b = {.type = TextureType::kTexture2DMultisample};
//     TextureDescriptor desc_c = {.type = TextureType::kTexture2D};

//     ASSERT_TRUE(desc_a.IsCompatibleWith(desc_b));
//     ASSERT_FALSE(desc_a.IsCompatibleWith(desc_c));
//   }
// }

// TEST(AllocatorTest, CachesTexturesWhenEnabled) {
//   auto allocator = std::make_shared<TestAllocator>();
//   auto desc = TextureDescriptor{
//       .format = PixelFormat::kR8G8B8A8UNormInt,
//       .size = ISize(100, 100),
//       .usage = static_cast<TextureUsageMask>(TextureUsage::kRenderTarget)};

//   allocator->DidAcquireSurfaceFrame();
//   allocator->CreateTexture(desc);

//   ASSERT_TRUE(allocator->GetCachedData().size() == 0);

//   allocator->SetEnableRenderTargetTextureCache(true);
//   allocator->CreateTexture(desc);

//   ASSERT_TRUE(allocator->GetCachedData().size() == 1);
// }

// TEST(AllocatorTest, DoesNotCacheNonRenderTargetTextures) {
//   auto allocator = std::make_shared<TestAllocator>();
//   auto desc = TextureDescriptor{
//       .format = PixelFormat::kR8G8B8A8UNormInt,
//       .size = ISize(100, 100),
//       .usage = static_cast<TextureUsageMask>(TextureUsage::kShaderRead)};

//   allocator->SetEnableRenderTargetTextureCache(true);
//   allocator->DidAcquireSurfaceFrame();
//   allocator->CreateTexture(desc);

//   ASSERT_TRUE(allocator->GetCachedData().size() == 0);
// }

// TEST(AllocatorTest, CachesUsedTexturesAcrossFrames) {
//   auto allocator = std::make_shared<TestAllocator>();
//   auto desc = TextureDescriptor{
//       .format = PixelFormat::kR8G8B8A8UNormInt,
//       .size = ISize(100, 100),
//       .usage = static_cast<TextureUsageMask>(TextureUsage::kRenderTarget)};

//   allocator->DidAcquireSurfaceFrame();
//   allocator->SetEnableRenderTargetTextureCache(true);
//   // Create two textures of the same exact size/shape. Both should be marked as
//   // used this frame, so the cached data set will contain two.
//   allocator->CreateTexture(desc);
//   allocator->CreateTexture(desc);

//   ASSERT_TRUE(allocator->GetCachedData().size() == 2);

//   allocator->DidFinishSurfaceFrame();
//   allocator->DidAcquireSurfaceFrame();

//   // Next frame, only create one texture. The set will still contain two,
//   // but one will be removed at the end of the frame.
//   allocator->CreateTexture(desc);
//   ASSERT_TRUE(allocator->GetCachedData().size() == 2);

//   allocator->DidFinishSurfaceFrame();
//   ASSERT_TRUE(allocator->GetCachedData().size() == 1);
// }

}  // namespace testing
}  // namespace impeller
