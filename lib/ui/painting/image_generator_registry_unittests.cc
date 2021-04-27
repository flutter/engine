// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_generator_registry.h"

#include "flutter/fml/mapping.h"
#include "flutter/shell/common/shell_test.h"
#include "flutter/testing/testing.h"

namespace flutter {
namespace testing {

static sk_sp<SkData> LoadValidImageFixture() {
  // Load the fixture with fml mappings.
  auto fixtures_directory =
      fml::OpenDirectory(GetFixturesPath(), false, fml::FilePermission::kRead);
  auto fixture_mapping = fml::FileMapping::CreateReadOnly(
      fixtures_directory, "DashInNooglerHat.jpg");

  // Remap to sk_sp<SkData>.
  SkData::ReleaseProc on_release = [](const void* ptr, void* context) -> void {
    delete reinterpret_cast<fml::FileMapping*>(context);
  };
  auto data = SkData::MakeWithProc(fixture_mapping->GetMapping(),
                                   fixture_mapping->GetSize(), on_release,
                                   fixture_mapping.get());
  fixture_mapping.release();

  return data;
}

TEST_F(ShellTest, CreateCompatibleReturnsBuiltinImageGeneratorForValidImage) {
  auto data = LoadValidImageFixture();

  // Fetch the generator and query for basic info
  ImageGeneratorRegistry registry;
  auto result = registry.createCompatible(data);
  auto info = result->getInfo();
  ASSERT_EQ(info.width(), 3024);
  ASSERT_EQ(info.height(), 4032);
}

TEST_F(ShellTest, CreateCompatibleReturnsNullptrForInvalidImage) {
  ImageGeneratorRegistry registry;
  auto result = registry.createCompatible(SkData::MakeEmpty());
  ASSERT_EQ(result, nullptr);
}

class FakeImageGenerator : public ImageGenerator {
 public:
  FakeImageGenerator(int identifiableFakeWidth)
      : info_(SkImageInfo::Make(identifiableFakeWidth,
                                identifiableFakeWidth,
                                SkColorType::kRGBA_8888_SkColorType,
                                SkAlphaType::kOpaque_SkAlphaType)){};
  ~FakeImageGenerator() = default;
  const SkImageInfo& getInfo() const { return info_; }

  bool getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) const {
    return false;
  };

  SkISize getScaledDimensions(float scale) {
    return SkISize::Make(info_.width(), info_.height());
  }

 private:
  SkImageInfo info_;
};

TEST_F(ShellTest, PositivePriorityTakesPrecedentOverDefaultGenerators) {
  ImageGeneratorRegistry registry;

  const int fakeWidth = 1337;
  registry.add(
      [fakeWidth](sk_sp<SkData> buffer) {
        return std::make_unique<FakeImageGenerator>(fakeWidth);
      },
      1);

  // Fetch the generator and query for basic info.
  auto result = registry.createCompatible(LoadValidImageFixture());
  ASSERT_EQ(result->getInfo().width(), fakeWidth);
}

TEST_F(ShellTest, DefaultGeneratorsTakePrecedentOverNegativePriority) {
  ImageGeneratorRegistry registry;

  registry.add(
      [](sk_sp<SkData> buffer) {
        return std::make_unique<FakeImageGenerator>(1337);
      },
      -1);

  // Fetch the generator and query for basic info.
  auto result = registry.createCompatible(LoadValidImageFixture());
  // If the real width of the image pops out, then the default generator was
  // returned rather than the fake one.
  ASSERT_EQ(result->getInfo().width(), 3024);
}

}  // namespace testing
}  // namespace flutter
