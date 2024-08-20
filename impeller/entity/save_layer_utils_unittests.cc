// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/entity/contents/filters/filter_contents.h"
#include "impeller/entity/save_layer_utils.h"

// TODO(zanderso): https://github.com/flutter/flutter/issues/127701
// NOLINTBEGIN(bugprone-unchecked-optional-access)

namespace impeller {
namespace testing {

using SaveLayerUtilsTest = ::testing::Test;

TEST(SaveLayerUtilsTest, SimplePaintComputedCoverage) {
  // Basic Case, simple paint, computed coverage
  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 10, 10),    //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/nullptr                              //
  );
  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

TEST(SaveLayerUtilsTest, BackdropFiterComputedCoverage) {
  // Backdrop Filter, computed coverage
  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 10, 10),    //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/nullptr,                             //
      /*has_backdrop_filter=*/true                          //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
}

TEST(SaveLayerUtilsTest, ImageFiterComputedCoverage) {
  // Image Filter, computed coverage
  auto image_filter = FilterContents::MakeMatrixFilter(
      FilterInput::Make(Rect()), Matrix::MakeScale({2, 2, 1}), {});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 10, 10),    //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/image_filter                         //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

TEST(SaveLayerUtilsTest,
     ImageFiterSmallScaleComputedCoverageLargerThanBoundsLimit) {
  // Image Filter scaling large, computed coverage is larger than bounds limit.
  auto image_filter = FilterContents::MakeMatrixFilter(
      FilterInput::Make(Rect()), Matrix::MakeScale({2, 2, 1}), {});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 10, 10),  //
      /*effect_transform=*/{},                            //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 5, 5),      //
      /*image_filter=*/image_filter                       //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2.5, 2.5));
}

TEST(SaveLayerUtilsTest,
     ImageFiterLargeScaleComputedCoverageLargerThanBoundsLimit) {
  // Image Filter scaling small, computed coverage is larger than bounds limit.
  auto image_filter = FilterContents::MakeMatrixFilter(
      FilterInput::Make(Rect()), Matrix::MakeScale({0.5, 0.5, 1}), {});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 10, 10),  //
      /*effect_transform=*/{},                            //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 5, 5),      //
      /*image_filter=*/image_filter                       //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

TEST(SaveLayerUtilsTest, DisjointCoverage) {
  // No intersection in coverage
  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(200, 200, 210, 210),  //
      /*effect_transform=*/{},                                  //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 100, 100),        //
      /*image_filter=*/nullptr                                  //
  );

  EXPECT_FALSE(coverage.has_value());
}

TEST(SaveLayerUtilsTest, DisjointCoverageTransformedByImageFilter) {
  // Coverage disjoint from parent coverage but transformed into parent space
  // with image filter.
  auto image_filter = FilterContents::MakeMatrixFilter(
      FilterInput::Make(Rect()), Matrix::MakeTranslation({-200, -200, 0}), {});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(200, 200, 210, 210),  //
      /*effect_transform=*/{},                                  //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 100, 100),        //
      /*image_filter=*/image_filter                             //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(200, 200, 210, 210));
}

TEST(SaveLayerUtilsTest, DisjointCoveragTransformedByCTM) {
  // Coverage disjoint from parent coverage.
  Matrix ctm = Matrix::MakeTranslation({-200, -200, 0});
  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(200, 200, 210, 210),  //
      /*effect_transform=*/ctm,                                 //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 100, 100),        //
      /*image_filter=*/nullptr                                  //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

TEST(SaveLayerUtilsTest, BasicEmptyCoverage) {
  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 0, 0),      //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/nullptr                              //
  );

  ASSERT_FALSE(coverage.has_value());
}

TEST(SaveLayerUtilsTest, ImageFilterEmptyCoverage) {
  // Empty coverage with Image Filter
  auto image_filter = FilterContents::MakeMatrixFilter(
      FilterInput::Make(Rect()), Matrix::MakeTranslation({-200, -200, 0}), {});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 0, 0),      //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/image_filter                         //
  );

  ASSERT_FALSE(coverage.has_value());
}

TEST(SaveLayerUtilsTest, BackdropFilterEmptyCoverage) {
  // Empty coverage with backdrop filter.
  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 0, 0),      //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/nullptr,                             //
      /*has_backdrop_filter=*/true                          //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
}


}  // namespace testing
}  // namespace impeller

// NOLINTEND(bugprone-unchecked-optional-access)
