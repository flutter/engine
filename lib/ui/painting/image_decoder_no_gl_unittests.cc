// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_decoder_no_gl_unittests.h"

namespace flutter {
namespace testing {

namespace {

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

float DecodeBGR10(uint32_t x) {
  const float max = 1.25098f;
  const float min = -0.752941f;
  const float intercept = min;
  const float slope = (max - min) / 1024.0f;
  return (x * slope) + intercept;
}

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
  ASSERT_EQ(wide_result->image_info.colorType(), kBGR_101010x_XR_SkColorType);
  ASSERT_TRUE(wide_result->image_info.colorSpace()->isSRGB());

  const SkPixmap& wide_pixmap = wide_result->sk_bitmap->pixmap();
  const uint32_t* pixel_ptr = static_cast<const uint32_t*>(wide_pixmap.addr());
  bool found_deep_red = false;
  for (int i = 0; i < wide_pixmap.width() * wide_pixmap.height(); ++i) {
    uint32_t pixel = *pixel_ptr++;
    float blue = DecodeBGR10((pixel >> 0) & 0x3ff);
    float green = DecodeBGR10((pixel >> 10) & 0x3ff);
    float red = DecodeBGR10((pixel >> 20) & 0x3ff);
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
