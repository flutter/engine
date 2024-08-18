// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/aiks/image_filter.h"
#include "impeller/aiks/save_layer_utils.h"
#include "impeller/geometry/color.h"

namespace impeller {
namespace testing {

using SaveLayerUtilsTest = ::testing::Test;

TEST(SaveLayerUtilsTest, SimplePaintComputedCoverage) {
  // Basic Case, simple paint, computed coverage
  auto coverage =
      ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 10, 10), nullptr, {}, {},
                               Rect::MakeLTRB(0, 0, 2400, 1800));
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

TEST(SaveLayerUtilsTest, DestructivePaintComputedCoverage) {
  // Destructive paint, computed coverage
  auto coverage = ComputeSaveLayerCoverage(
      Rect::MakeLTRB(0, 0, 10, 10), nullptr, {.blend_mode = BlendMode::kClear},
      {}, Rect::MakeLTRB(0, 0, 2400, 1800));
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
}

TEST(SaveLayerUtilsTest, BackdropFiterComputedCoverage) {
  // Backdrop Filter, computed coverage
  auto backdrop_filter =
      ImageFilter::MakeMatrix(Matrix::MakeScale({2, 2, 1}), {});
  auto coverage =
      ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 10, 10), backdrop_filter,
                               {}, {}, Rect::MakeLTRB(0, 0, 2400, 1800));
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
}

TEST(SaveLayerUtilsTest, ImageFiterComputedCoverage) {
  // Backdrop Filter, computed coverage
  auto image_filter = ImageFilter::MakeMatrix(Matrix::MakeScale({2, 2, 1}), {});
  auto coverage = ComputeSaveLayerCoverage(
      Rect::MakeLTRB(0, 0, 10, 10), nullptr, {.image_filter = image_filter}, {},
      Rect::MakeLTRB(0, 0, 2400, 1800));
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

TEST(SaveLayerUtilsTest,
     ImageFiterSmallScaleComputedCoverageLargerThanBoundsLimit) {
  // Image Filter scaling large, computed coverage is larger than bounds limit.
  auto image_filter = ImageFilter::MakeMatrix(Matrix::MakeScale({2, 2, 1}), {});
  auto coverage = ComputeSaveLayerCoverage(
      Rect::MakeLTRB(0, 0, 10, 10), nullptr, {.image_filter = image_filter}, {},
      Rect::MakeLTRB(0, 0, 5, 5));
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2.5, 2.5));
}

TEST(SaveLayerUtilsTest,
     ImageFiterLargeScaleComputedCoverageLargerThanBoundsLimit) {
  // Image Filter scaling small, computed coverage is larger than bounds limit.
  auto image_filter =
      ImageFilter::MakeMatrix(Matrix::MakeScale({0.5, 0.5, 1}), {});
  auto coverage = ComputeSaveLayerCoverage(
      Rect::MakeLTRB(0, 0, 10, 10), nullptr, {.image_filter = image_filter}, {},
      Rect::MakeLTRB(0, 0, 5, 5));
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

TEST(SaveLayerUtilsTest, DisjointCoverage) {
  // No intersection in coverage
  auto coverage =
      ComputeSaveLayerCoverage(Rect::MakeLTRB(200, 200, 210, 210), nullptr, {},
                               {}, Rect::MakeLTRB(0, 0, 100, 100));
  EXPECT_FALSE(coverage.has_value());
}

TEST(SaveLayerUtilsTest, DisjointCoverageTransformedByImageFilter) {
  // Coverage disjoint from parent coverage but transformed into parent space
  // with image filter.
  auto image_filter =
      ImageFilter::MakeMatrix(Matrix::MakeTranslation({-200, -200, 0}), {});
  auto coverage = ComputeSaveLayerCoverage(
      Rect::MakeLTRB(200, 200, 210, 210), nullptr,
      {.image_filter = image_filter}, {}, Rect::MakeLTRB(0, 0, 100, 100));
  ASSERT_TRUE(coverage.has_value());
  // Is this the right value? should it actually be (0, 0, 10, 10)?
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(200, 200, 210, 210));
}

TEST(SaveLayerUtilsTest, DisjointCoverageNotTransformedByCTM) {
  // Coverage disjoint from parent coverage. CTM does not impact lack of
  // intersection as it has already been "absorbed" by child coverage.
  Matrix ctm = Matrix::MakeTranslation({-200, -200, 0});
  auto coverage =
      ComputeSaveLayerCoverage(Rect::MakeLTRB(200, 200, 210, 210), nullptr, {},
                               ctm, Rect::MakeLTRB(0, 0, 100, 100));
  ASSERT_FALSE(coverage.has_value());
}

TEST(SaveLayerUtilsTest, BasicEmptyCoverage) {
  auto coverage =
      ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 0, 0), nullptr, {}, {},
                               Rect::MakeLTRB(0, 0, 2400, 1800));
  ASSERT_FALSE(coverage.has_value());
}

TEST(SaveLayerUtilsTest, ImageFilterEmptyCoverage) {
  // Empty coverage with Image Filter
  auto image_filter =
      ImageFilter::MakeMatrix(Matrix::MakeTranslation({-200, -200, 0}), {});
  auto coverage = ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 0, 0), nullptr,
                                           {.image_filter = image_filter}, {},
                                           Rect::MakeLTRB(0, 0, 2400, 1800));
  ASSERT_FALSE(coverage.has_value());
}

TEST(SaveLayerUtilsTest, BackdropFilterEmptyCoverage) {
  // Empty coverage with Image Filter
  auto image_filter =
      ImageFilter::MakeMatrix(Matrix::MakeTranslation({-200, -200, 0}), {});
  auto coverage =
      ComputeSaveLayerCoverage(Rect::MakeLTRB(0, 0, 0, 0), image_filter, {}, {},
                               Rect::MakeLTRB(0, 0, 2400, 1800));
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
}

TEST(SaveLayerUtilsTest, DestructivePaintUserSpecifiedBounds) {
  // user specified bounds override clip flooding.
  auto coverage = ComputeSaveLayerCoverage(
      Rect::MakeLTRB(0, 0, 10, 10), nullptr, {.blend_mode = BlendMode::kClear},
      {}, Rect::MakeLTRB(0, 0, 2400, 1800),
      /*user_provided_bounds=*/true);
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

TEST(SaveLayerUtilsTest, BackdropFilterUserSpecifiedBounds) {
  // user specified bounds override clip flooding.
  auto backdrop_filter =
      ImageFilter::MakeMatrix(Matrix::MakeScale({2, 2, 1}), {});
  auto coverage = ComputeSaveLayerCoverage(
      Rect::MakeLTRB(0, 0, 10, 10), backdrop_filter, {}, {},
      Rect::MakeLTRB(0, 0, 2400, 1800), /*user_provided_bounds=*/true);
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

}  // namespace testing
}  // namespace impeller
