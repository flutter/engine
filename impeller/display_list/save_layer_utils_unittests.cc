// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display_list/dl_sampling_options.h"
#include "display_list/effects/dl_image_filter.h"
#include "flutter/testing/testing.h"
#include "impeller/display_list/save_layer_utils.h"
#include "include/core/SkMatrix.h"

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
      /*image_filter=*/nullptr,
      /*flood_output_coverage=*/false,  //
      /*flood_input_coverage=*/true     //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
}

TEST(SaveLayerUtilsTest, ImageFiterComputedCoverage) {
  // Image Filter, computed coverage
  SkMatrix matrix;
  matrix.setScale(2, 2);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 10, 10),    //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/image_filter.get()                   //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 10, 10));
}

TEST(SaveLayerUtilsTest,
     ImageFiterSmallScaleComputedCoverageLargerThanBoundsLimit) {
  // Image Filter scaling large, computed coverage is larger than bounds limit.
  SkMatrix matrix;
  matrix.setScale(2, 2);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 10, 10),  //
      /*effect_transform=*/{},                            //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 5, 5),      //
      /*image_filter=*/image_filter.get()                 //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 3, 3));
}

TEST(SaveLayerUtilsTest,
     ImageFiterLargeScaleComputedCoverageLargerThanBoundsLimit) {
  // Image Filter scaling small, computed coverage is larger than bounds limit.
  SkMatrix matrix;
  matrix.setScale(0.5, 0.5);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 10, 10),  //
      /*effect_transform=*/{},                            //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 5, 5),      //
      /*image_filter=*/image_filter.get()                 //
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
  SkMatrix matrix = SkMatrix::Translate(-200, -200);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(200, 200, 210, 210),  //
      /*effect_transform=*/{},                                  //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 100, 100),        //
      /*image_filter=*/image_filter.get()                       //
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
  SkMatrix matrix = SkMatrix::Translate(-200, -200);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 0, 0),      //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/image_filter.get()                   //
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
      /*flood_output_coverage=*/true                        //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
}

TEST(SaveLayerUtilsTest, FloodInputCoverage) {
  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 0, 0),      //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/nullptr,                             //
      /*flood_output_coverage=*/false,                      //
      /*flood_input_coverage=*/true                         //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 2400, 1800));
}

TEST(SaveLayerUtilsTest, FloodInputCoverageWithImageFilter) {
  SkMatrix matrix = SkMatrix::Scale(0.5, 0.5);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 0, 0),      //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/image_filter.get(),                  //
      /*flood_output_coverage=*/false,                      //
      /*flood_input_coverage=*/true                         //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 4800, 3600));
}

TEST(SaveLayerUtilsTest,
     FloodInputCoverageWithImageFilterWithNoCoverageProducesNoCoverage) {
  // Even if we flood the input coverage due to a bdf, we can still cull out the
  // layer if the image filter results in no coverage.
  SkMatrix matrix = SkMatrix::Scale(1, 0);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 0, 0),      //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 2400, 1800),  //
      /*image_filter=*/image_filter.get(),                  //
      /*flood_output_coverage=*/false,                      //
      /*flood_input_coverage=*/true                         //
  );

  ASSERT_FALSE(coverage.has_value());
}

TEST(
    SaveLayerUtilsTest,
    CoverageLimitIgnoredIfIntersectedValueIsCloseToActualCoverageSmallerWithImageFilter) {
  // Create an image filter that slightly shrinks the coverage limit
  SkMatrix matrix = SkMatrix::Scale(1.1, 1.1);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 100, 100),  //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 100, 100),    //
      /*image_filter=*/image_filter.get()                   //
  );

  ASSERT_TRUE(coverage.has_value());
  // The transfomed coverage limit is ((0, 0), (90.9091, 90.9091)).
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 100, 100));
}

TEST(
    SaveLayerUtilsTest,
    CoverageLimitIgnoredIfIntersectedValueIsCloseToActualCoverageLargerWithImageFilter) {
  // Create an image filter that slightly stretches the coverage limit. Even
  // without the special logic for using the original content coverage, we
  // verify that we don't introduce any artifacts from the intersection.
  SkMatrix matrix = SkMatrix::Scale(0.9, 0.9);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 100, 100),  //
      /*effect_transform=*/{},                              //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 100, 100),    //
      /*image_filter=*/image_filter.get()                   //
  );

  ASSERT_TRUE(coverage.has_value());
  // The transfomed coverage limit is ((0, 0), (111.111, 111.111)).
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 100, 100));
}

TEST(SaveLayerUtilsTest,
     CoverageLimitRespectedIfSubstantiallyDifferentFromContentCoverge) {
  SkMatrix matrix = SkMatrix::Scale(2, 2);
  auto image_filter =
      flutter::DlMatrixImageFilter::Make(matrix, flutter::DlImageSampling{});

  auto coverage = ComputeSaveLayerCoverage(
      /*content_coverage=*/Rect::MakeLTRB(0, 0, 1000, 1000),  //
      /*effect_transform=*/{},                                //
      /*coverage_limit=*/Rect::MakeLTRB(0, 0, 100, 100),      //
      /*image_filter=*/image_filter.get()                     //
  );

  ASSERT_TRUE(coverage.has_value());
  EXPECT_EQ(coverage.value(), Rect::MakeLTRB(0, 0, 50, 50));
}

}  // namespace testing
}  // namespace impeller

// NOLINTEND(bugprone-unchecked-optional-access)
