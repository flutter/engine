// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "flutter/impeller/core/allocator.h"
#include "flutter/impeller/core/device_buffer.h"
#include "flutter/impeller/core/formats.h"
#include "flutter/impeller/geometry/size.h"
#include "flutter/lib/ui/painting/image_decoder.h"
#include "flutter/lib/ui/painting/image_decoder_impeller.h"
#include "flutter/testing/testing.h"

namespace impeller {

class TestImpellerTexture : public Texture {
 public:
  explicit TestImpellerTexture(TextureDescriptor desc) : Texture(desc) {}

  void SetLabel(std::string_view label) override {}
  bool IsValid() const override { return true; }
  ISize GetSize() const { return GetTextureDescriptor().size; }

  bool OnSetContents(const uint8_t* contents, size_t length, size_t slice) {
    return true;
  }
  bool OnSetContents(std::shared_ptr<const fml::Mapping> mapping,
                     size_t slice) {
    return true;
  }
};

class TestImpellerDeviceBuffer : public DeviceBuffer {
 public:
  explicit TestImpellerDeviceBuffer(DeviceBufferDescriptor desc)
      : DeviceBuffer(desc) {
    bytes_ = static_cast<uint8_t*>(malloc(desc.size));
  }

  ~TestImpellerDeviceBuffer() { free(bytes_); }

 private:
  std::shared_ptr<Texture> AsTexture(Allocator& allocator,
                                     const TextureDescriptor& descriptor,
                                     uint16_t row_bytes) const override {
    return nullptr;
  }

  bool SetLabel(const std::string& label) override { return true; }

  bool SetLabel(const std::string& label, Range range) override { return true; }

  uint8_t* OnGetContents() const override { return bytes_; }

  bool OnCopyHostBuffer(const uint8_t* source,
                        Range source_range,
                        size_t offset) override {
    for (auto i = source_range.offset; i < source_range.length; i++, offset++) {
      bytes_[offset] = source[i];
    }
    return true;
  }

  uint8_t* bytes_;
};

class TestImpellerAllocator : public impeller::Allocator {
 public:
  TestImpellerAllocator() {}

  ~TestImpellerAllocator() = default;

 private:
  uint16_t MinimumBytesPerRow(PixelFormat format) const override { return 0; }

  ISize GetMaxTextureSizeSupported() const override {
    return ISize{2048, 2048};
  }

  std::shared_ptr<DeviceBuffer> OnCreateBuffer(
      const DeviceBufferDescriptor& desc) override {
    return std::make_shared<TestImpellerDeviceBuffer>(desc);
  }

  std::shared_ptr<Texture> OnCreateTexture(
      const TextureDescriptor& desc) override {
    return std::make_shared<TestImpellerTexture>(desc);
  }
};

}  // namespace impeller

namespace flutter {
namespace testing {

namespace {

float HalfToFloat(uint16_t half) {
  switch (half) {
    case 0x7c00:
      return std::numeric_limits<float>::infinity();
    case 0xfc00:
      return -std::numeric_limits<float>::infinity();
  }
  bool negative = half >> 15;
  uint16_t exponent = (half >> 10) & 0x1f;
  uint16_t fraction = half & 0x3ff;
  float fExponent = exponent - 15.0f;
  float fFraction = static_cast<float>(fraction) / 1024.f;
  float pow_value = powf(2.0f, fExponent);
  return (negative ? -1.f : 1.f) * pow_value * (1.0f + fFraction);
}

bool IsPngWithPLTE(const uint8_t* bytes, size_t size) {
  if (size < 8) {
    return false;
  }

  if (memcmp(bytes, "\x89PNG\x0d\x0a\x1a\x0a", 8) != 0) {
    return false;
  }

  const uint8_t* end = bytes + size;
  const uint8_t* loc = bytes + 8;
  while (loc + 8 <= end) {
    uint32_t chunk_length =
        (loc[0] << 24) | (loc[1] << 16) | (loc[2] << 8) | loc[3];

    if (memcmp(loc + 4, "PLTE", 4) == 0) {
      return true;
    }

    loc += /*length*/ 4 + /*type*/ 4 + chunk_length + /*crc*/ 4;
  }

  return false;
}

sk_sp<SkData> OpenFixtureAsSkData(const char* name) {
  auto fixtures_directory =
      fml::OpenDirectory(GetFixturesPath(), false, fml::FilePermission::kRead);
  if (!fixtures_directory.is_valid()) {
    return nullptr;
  }

  auto fixture_mapping =
      fml::FileMapping::CreateReadOnly(fixtures_directory, name);

  if (!fixture_mapping) {
    return nullptr;
  }

  SkData::ReleaseProc on_release = [](const void* ptr, void* context) -> void {
    delete reinterpret_cast<fml::FileMapping*>(context);
  };

  auto data = SkData::MakeWithProc(fixture_mapping->GetMapping(),
                                   fixture_mapping->GetSize(), on_release,
                                   fixture_mapping.get());

  if (!data) {
    return nullptr;
  }
  // The data is now owned by Skia.
  fixture_mapping.release();
  return data;
}

}  // namespace

TEST(ImageDecoderNoGLTest, ImpellerWideGamutDisplayP3) {
  auto data = OpenFixtureAsSkData("DisplayP3Logo.png");
  auto image = SkImages::DeferredFromEncodedData(data);
  ASSERT_TRUE(image != nullptr);
  ASSERT_EQ(SkISize::Make(100, 100), image->dimensions());

  ImageGeneratorRegistry registry;
  std::shared_ptr<ImageGenerator> generator =
      registry.CreateCompatibleGenerator(data);
  ASSERT_TRUE(generator);

  auto descriptor = fml::MakeRefCounted<ImageDescriptor>(std::move(data),
                                                         std::move(generator));

  ASSERT_FALSE(
      IsPngWithPLTE(descriptor->data()->bytes(), descriptor->data()->size()));

#if IMPELLER_SUPPORTS_RENDERING
  std::shared_ptr<impeller::Allocator> allocator =
      std::make_shared<impeller::TestImpellerAllocator>();
  std::optional<DecompressResult> wide_result =
      ImageDecoderImpeller::DecompressTexture(
          descriptor.get(), SkISize::Make(100, 100), {100, 100},
          /*supports_wide_gamut=*/true, allocator);
  ASSERT_TRUE(wide_result.has_value());
  ASSERT_EQ(wide_result->image_info.colorType(), kRGBA_F16_SkColorType);
  ASSERT_TRUE(wide_result->image_info.colorSpace()->isSRGB());

  const SkPixmap& wide_pixmap = wide_result->sk_bitmap->pixmap();
  const uint16_t* half_ptr = static_cast<const uint16_t*>(wide_pixmap.addr());
  bool found_deep_red = false;
  for (int i = 0; i < wide_pixmap.width() * wide_pixmap.height(); ++i) {
    float red = HalfToFloat(*half_ptr++);
    float green = HalfToFloat(*half_ptr++);
    float blue = HalfToFloat(*half_ptr++);
    half_ptr++;  // alpha
    if (fabsf(red - 1.0931f) < 0.01f && fabsf(green - -0.2268f) < 0.01f &&
        fabsf(blue - -0.1501f) < 0.01f) {
      found_deep_red = true;
      break;
    }
  }

  ASSERT_TRUE(found_deep_red);
  std::optional<DecompressResult> narrow_result =
      ImageDecoderImpeller::DecompressTexture(
          descriptor.get(), SkISize::Make(100, 100), {100, 100},
          /*supports_wide_gamut=*/false, allocator);

  ASSERT_TRUE(narrow_result.has_value());
  ASSERT_EQ(narrow_result->image_info.colorType(), kRGBA_8888_SkColorType);
#endif  // IMPELLER_SUPPORTS_RENDERING
}

TEST(ImageDecoderNoGLTest, ImpellerWideGamutIndexedPng) {
  auto data = OpenFixtureAsSkData("WideGamutIndexed.png");
  auto image = SkImages::DeferredFromEncodedData(data);
  ASSERT_TRUE(image != nullptr);
  ASSERT_EQ(SkISize::Make(100, 100), image->dimensions());

  ImageGeneratorRegistry registry;
  std::shared_ptr<ImageGenerator> generator =
      registry.CreateCompatibleGenerator(data);
  ASSERT_TRUE(generator);

  auto descriptor = fml::MakeRefCounted<ImageDescriptor>(std::move(data),
                                                         std::move(generator));

  ASSERT_TRUE(
      IsPngWithPLTE(descriptor->data()->bytes(), descriptor->data()->size()));

#if IMPELLER_SUPPORTS_RENDERING
  std::shared_ptr<impeller::Allocator> allocator =
      std::make_shared<impeller::TestImpellerAllocator>();
  std::optional<DecompressResult> wide_result =
      ImageDecoderImpeller::DecompressTexture(
          descriptor.get(), SkISize::Make(100, 100), {100, 100},
          /*supports_wide_gamut=*/true, allocator);
  ASSERT_EQ(wide_result->image_info.colorType(), kRGBA_F16_SkColorType);
  ASSERT_TRUE(wide_result->image_info.colorSpace()->isSRGB());

  const SkPixmap& wide_pixmap = wide_result->sk_bitmap->pixmap();
  const uint16_t* half_ptr = static_cast<const uint16_t*>(wide_pixmap.addr());
  bool found_deep_red = false;
  for (int i = 0; i < wide_pixmap.width() * wide_pixmap.height(); ++i) {
    float red = HalfToFloat(*half_ptr++);
    float green = HalfToFloat(*half_ptr++);
    float blue = HalfToFloat(*half_ptr++);
    half_ptr++;  // alpha
    if (fabsf(red - 1.0931f) < 0.01f && fabsf(green - -0.2268f) < 0.01f &&
        fabsf(blue - -0.1501f) < 0.01f) {
      found_deep_red = true;
      break;
    }
  }

  ASSERT_TRUE(found_deep_red);
  std::optional<DecompressResult> narrow_result =
      ImageDecoderImpeller::DecompressTexture(
          descriptor.get(), SkISize::Make(100, 100), {100, 100},
          /*supports_wide_gamut=*/false, allocator);

  ASSERT_TRUE(narrow_result.has_value());
  ASSERT_EQ(narrow_result->image_info.colorType(), kRGBA_8888_SkColorType);
#endif  // IMPELLER_SUPPORTS_RENDERING
}

}  // namespace testing
}  // namespace flutter
