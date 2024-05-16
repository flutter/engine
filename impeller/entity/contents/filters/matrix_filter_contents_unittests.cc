// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "gmock/gmock.h"
#include "impeller/entity/contents/filters/matrix_filter_contents.h"

namespace impeller {
namespace testing {
TEST(MatrixFilterContentsTest, Create) {
  MatrixFilterContents contents;
  EXPECT_TRUE(contents.IsTranslationOnly());
}

TEST(MatrixFilterContentsTest, CoverageEmpty) {
  MatrixFilterContents contents;
  FilterInput::Vector inputs = {};
  Entity entity;
  std::optional<Rect> coverage =
      contents.GetFilterCoverage(inputs, entity, /*effect_transform=*/Matrix());
  ASSERT_FALSE(coverage.has_value());
}

TEST(MatrixFilterContentsTest, CoverageSimple) {
  MatrixFilterContents contents;
  FilterInput::Vector inputs = {
      FilterInput::Make(Rect::MakeLTRB(10, 10, 110, 110))};
  Entity entity;
  std::optional<Rect> coverage =
      contents.GetFilterCoverage(inputs, entity, /*effect_transform=*/Matrix());

  ASSERT_EQ(coverage, Rect::MakeLTRB(10, 10, 110, 110));
}

TEST(MatrixFilterContentsTest, Coverage2x) {
  MatrixFilterContents contents;
  contents.SetMatrix(Matrix::MakeScale({2.0, 2.0, 1.0}));
  FilterInput::Vector inputs = {
      FilterInput::Make(Rect::MakeXYWH(10, 10, 100, 100))};
  Entity entity;
  std::optional<Rect> coverage =
      contents.GetFilterCoverage(inputs, entity, /*effect_transform=*/Matrix());

  ASSERT_EQ(coverage, Rect::MakeXYWH(20, 20, 200, 200));
}

TEST(MatrixFilterContentsTest, Coverage2xEffect) {
  MatrixFilterContents contents;
  FilterInput::Vector inputs = {
      FilterInput::Make(Rect::MakeXYWH(10, 10, 100, 100))};
  Entity entity;
  std::optional<Rect> coverage = contents.GetFilterCoverage(
      inputs, entity, /*effect_transform=*/Matrix::MakeScale({2.0, 2.0, 1.0}));

  ASSERT_EQ(coverage, Rect::MakeXYWH(10, 10, 100, 100));
}

}  // namespace testing
}  // namespace impeller
