// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "gmock/gmock.h"
#include "impeller/entity/contents/filters/gaussian_blur_filter_contents.h"

namespace impeller {

using testing::Return;

namespace {

class MockTexture : public Texture {
 public:
  MockTexture(TextureDescriptor desc) : Texture(desc) {}

  MOCK_METHOD(void, SetLabel, (std::string_view label), (override));
  MOCK_METHOD(bool, IsValid, (), (const, override));
  MOCK_METHOD(ISize, GetSize, (), (const, override));
  MOCK_METHOD(bool,
              OnSetContents,
              (const uint8_t* contents, size_t length, size_t slice),
              (override));
  MOCK_METHOD(bool,
              OnSetContents,
              (std::shared_ptr<const fml::Mapping> mapping, size_t slice),
              (override));
};

Scalar CalculateSigmaForBlurRadius(Scalar blur_radius) {
  // See Sigma.h
  return (blur_radius / kKernelRadiusPerSigma) + 0.5;
}
}  // namespace

TEST(GaussianBlurFilterContents, Create) {
  GaussianBlurFilterContents contents;
  ASSERT_EQ(contents.GetSigma(), 0.0);
}

TEST(GaussianBlurFilterContents, CoverageEmpty) {
  GaussianBlurFilterContents contents;
  FilterInput::Vector inputs = {};
  Entity entity;
  std::optional<Rect> coverage =
      contents.GetFilterCoverage(inputs, entity, /*effect_transform=*/Matrix());
  ASSERT_FALSE(coverage.has_value());
}

TEST(GaussianBlurFilterContents, CoverageSimple) {
  GaussianBlurFilterContents contents;
  FilterInput::Vector inputs = {
      FilterInput::Make(Rect::MakeLTRB(10, 10, 110, 110))};
  Entity entity;
  std::optional<Rect> coverage =
      contents.GetFilterCoverage(inputs, entity, /*effect_transform=*/Matrix());
  ASSERT_EQ(coverage, Rect::MakeLTRB(10, 10, 110, 110));
}

TEST(GaussianBlurFilterContents, CoverageWithSigma) {
  Scalar sigma_radius_1 = CalculateSigmaForBlurRadius(1.0);
  GaussianBlurFilterContents contents(/*sigma=*/sigma_radius_1);
  FilterInput::Vector inputs = {
      FilterInput::Make(Rect::MakeLTRB(100, 100, 200, 200))};
  Entity entity;
  std::optional<Rect> coverage =
      contents.GetFilterCoverage(inputs, entity, /*effect_transform=*/Matrix());
  ASSERT_EQ(coverage, Rect::MakeLTRB(99, 99, 201, 201));
}

TEST(GaussianBlurFilterContents, CoverageWithTexture) {
  TextureDescriptor desc = {
      .size = ISize(100, 100),
  };
  Scalar sigma_radius_1 = CalculateSigmaForBlurRadius(1.0);
  GaussianBlurFilterContents contents(/*sigma=*/sigma_radius_1);
  std::shared_ptr<MockTexture> texture = std::make_shared<MockTexture>(desc);
  EXPECT_CALL(*texture, GetSize()).WillRepeatedly(Return(ISize(100, 100)));
  FilterInput::Vector inputs = {FilterInput::Make(texture)};
  Entity entity;
  entity.SetTransformation(Matrix::MakeTranslation({100, 100, 0}));
  std::optional<Rect> coverage =
      contents.GetFilterCoverage(inputs, entity, /*effect_transform=*/Matrix());
  ASSERT_EQ(coverage, Rect::MakeLTRB(99, 99, 201, 201));
}

TEST(GaussianBlurFilterContents, CoverageWithEffectTransform) {
  TextureDescriptor desc = {
      .size = ISize(100, 100),
  };
  Scalar sigma_radius_1 = CalculateSigmaForBlurRadius(1.0);
  GaussianBlurFilterContents contents(/*sigma=*/sigma_radius_1);
  std::shared_ptr<MockTexture> texture = std::make_shared<MockTexture>(desc);
  EXPECT_CALL(*texture, GetSize()).WillRepeatedly(Return(ISize(100, 100)));
  FilterInput::Vector inputs = {FilterInput::Make(texture)};
  Entity entity;
  entity.SetTransformation(Matrix::MakeTranslation({100, 100, 0}));
  std::optional<Rect> coverage = contents.GetFilterCoverage(
      inputs, entity, /*effect_transform=*/Matrix::MakeScale({2.0, 2.0, 1.0}));
  ASSERT_EQ(coverage, Rect::MakeLTRB(100 - 2, 100 - 2, 200 + 2, 200 + 2));
}

TEST(GaussianBlurFilterContents, FilterSourceCoverage) {
  Scalar sigma_radius_1 = CalculateSigmaForBlurRadius(1.0);
  auto contents = std::make_unique<GaussianBlurFilterContents>(sigma_radius_1);
  std::optional<Rect> coverage = contents->GetFilterSourceCoverage(
      /*effect_transform=*/Matrix::MakeScale({2.0, 2.0, 1.0}),
      /*output_limit=*/Rect::MakeLTRB(100, 100, 200, 200));
  ASSERT_EQ(coverage, Rect::MakeLTRB(100 - 2, 100 - 2, 200 + 2, 200 + 2));
}

}  // namespace impeller
